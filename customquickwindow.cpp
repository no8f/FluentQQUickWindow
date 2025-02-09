//-------------------------------------------------------
//
//           SPDX-License-Identifier: LGPLv2
//
//             Copyright © 2024 Noah Felber
//
//      All Rights Reserved/Alle Rechte vorbehalten.
//
//-------------------------------------------------------

#include "customquickwindow.h"

#include <QQuickItem>
#include <QSettings>
#include <QOperatingSystemVersion>
#include <QAbstractNativeEventFilter>
#include <QEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QCursor>
#include <cassert>
#ifdef Q_OS_WIN

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <QApplication>
#include <QWidget>

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif
#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#endif // Q_OS_WIN

class WinEventFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override {
        if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
            MSG *msg = static_cast<MSG *>(message);

            // Titelbar-Interaktion abfangen
            if (msg->message == WM_CREATE) {
                RECT client;
                GetWindowRect(hWnd, &client);

                SetWindowPos(hWnd, NULL, client.left, client.top, client.right - client.left,
                             client.bottom - client.top, SWP_FRAMECHANGED);

                return true;
            }

            if (msg->message == WM_NCCALCSIZE) {
                if ( msg->wParam == TRUE ) {
                    NCCALCSIZE_PARAMS* pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);

                    pncsp->rgrc[0].top += 12;
                    // **Kompletten Fensterrahmen entfernen**
                    *result = 0;
                    return true;
                }
            }

            if ( msg->message == WM_NCHITTEST ) {

                QPoint cursorPos = QCursor::pos();
                QPoint localPos = window->mapFromGlobal(cursorPos);

                // **Prüfen, ob die Maus über einem UI-Element ist**
                QQuickItem *widgetUnderMouse = window->contentItem()->childAt(localPos.x(), localPos.y());
                if (widgetUnderMouse) {
                    *result = HTCLIENT;  // Button ist klickbar!
                    return true;
                }

                RECT winRect;
                GetWindowRect(msg->hwnd, &winRect);
                GetWindowRect(msg->hwnd, &winRect);

                int x = GET_X_LPARAM(msg->lParam);
                int y = GET_Y_LPARAM(msg->lParam);

                bool left = x < winRect.left + BORDER_THICKNESS;
                bool right = x > winRect.right - BORDER_THICKNESS;
                bool top = y < winRect.top + BORDER_THICKNESS;
                bool bottom = y > winRect.bottom - BORDER_THICKNESS;

                // **Ecken für Resizing**
                if (top && left) { *result = HTTOPLEFT; return true; }
                if (top && right) { *result = HTTOPRIGHT; return true; }
                if (bottom && left) { *result = HTBOTTOMLEFT; return true; }
                if (bottom && right) { *result = HTBOTTOMRIGHT; return true; }

                // **Kanten für Resizing**
                if (left) { *result = HTLEFT; return true; }
                if (right) { *result = HTRIGHT; return true; }
                if (top) { *result = HTTOP; return true; }
                if (bottom) { *result = HTBOTTOM; return true; }

                const int TITLEBAR_HEIGHT = 45;

                // **Überprüfen, ob der Klick in der Titelleiste war**
                if (y < winRect.top + TITLEBAR_HEIGHT) {
                    *result = HTCAPTION; // **Fenster draggable machen**
                    return true;
                }

                return false;
            }
        }
        return false;
    }

    void setHWnd(HWND newHWnd) {
        hWnd = newHWnd;
    };

    void setWindow(QPointer<QQuickWindow> win) {
        window = win;
    };

private:
    HWND hWnd;
    const int BORDER_THICKNESS = 8;
    QPointer<QQuickWindow> window;
};


class CustomQtEventFilter : public QObject {
    Q_OBJECT

public:
    CustomQtEventFilter(QObject* target) : QObject(), _target(target) {}

    bool eventFilter(QObject *obj, QEvent *event) override {
        if ( obj == _target && ( event->type() == QEvent::Close || event->type() == QEvent::Quit ) ) {
            QQuickWindow *window = qobject_cast<QQuickWindow *>(_target);
            QSettings().setValue("CustomQQuickWindow/geometry", window->geometry());
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QObject *_target;
};

#include "customquickwindow.moc"

class CustomQQuickWindowPriv {
    friend class CustomQQuickWindow;

    bool _mousePressed = false, _globalDrag = false, _curDragging = false, _systemResizeStarted = false;
    int _resizeEdges = 0, _resizeAreaMargin = 10, _titleBarHeight = 30;
    QPointF _lastMousePos, _lastWindowPosition;
    QRect _startGeometry;

    std::unique_ptr<CustomQtEventFilter> _eventFilter;
    std::unique_ptr<WinEventFilter> _winNativeEventFilter;
};

CustomQQuickWindow::CustomQQuickWindow(QWindow *parent)
    : QQuickWindow(parent) {

    _p = new CustomQQuickWindowPriv;

#ifdef Q_OS_WIN
    HWND hwnd = (HWND)winId();

    BOOL v = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &v, sizeof(v));

    MARGINS margins = { 0, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    _p->_winNativeEventFilter = std::make_unique<WinEventFilter>();
    _p->_winNativeEventFilter->setHWnd(hwnd);
    _p->_winNativeEventFilter->setWindow(this);

    qApp->installNativeEventFilter(_p->_winNativeEventFilter.get());

    LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~(WS_SYSMENU);
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    // **Fenster-Style aktualisieren**
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
#endif

    _p->_eventFilter = std::make_unique<CustomQtEventFilter>(this);
    this->installEventFilter(_p->_eventFilter.get());
}

const void CustomQQuickWindow::setGlobalDrag(const bool enable)
{
    _p->_globalDrag = enable;
}

const bool CustomQQuickWindow::globalDrag()
{
    return _p->_globalDrag;
}

const void CustomQQuickWindow::setResizeMargin(const int margin)
{
    _p->_resizeAreaMargin = margin;
}

const int CustomQQuickWindow::resizeMargin()
{
    return _p->_resizeAreaMargin;
}

const void CustomQQuickWindow::setTitleBarHeight(const int height)
{
    _p->_titleBarHeight = height;
}

const int CustomQQuickWindow::titleBarHeight()
{
    return _p->_titleBarHeight;
}

const void CustomQQuickWindow::registerQmlType()
{
#ifndef CUSTOMQML_URI
#define CUSTOMQML_URI "FluentQQUickWindow"
#endif

#ifdef Q_OS_WIN
    assert((void("Incopatible Windows version"), QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11_22H2));
#endif

    qmlRegisterType<CustomQQuickWindow>(CUSTOMQML_URI, 1, 0, "CustomQQuickWindow");
}
