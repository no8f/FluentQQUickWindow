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
#include <winuser.h>
#include <uxtheme.h>
#include <dwmapi.h>

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
// class CustomNativeEventFilter : public QAbstractNativeEventFilter {
// public:
//     CustomNativeEventFilter(QQuickWindow* targetWidget)
//         : m_targetWidget(targetWidget),
//         m_isResizeEnabled(true),
//         m_borderWidth(8) {}

//     bool nativeEventFilter(const QByteArray &eventType, void *message, long long *result) override {
//         MSG* msg = static_cast<MSG*>(message);

//         if (!msg->hwnd) {
//             return false;
//         }

//         // Resize handling
//         if (msg->message == WM_NCHITTEST && m_isResizeEnabled) {
//             return handleResizeHitTest(msg, result);
//         }

//         // Custom title bar button handling
//         if (handleTitleBarButtons(msg)) {
//             *result = 0;
//             return true;
//         }

//         // NC Calc Size handling for maximized/fullscreen windows
//         if (msg->message == WM_NCCALCSIZE) {
//             return handleNCCalcSize(msg, result);
//         }

//         return false;
//     }

// private:
//     QQuickWindow* m_targetWidget;
//     bool m_isResizeEnabled;
//     int m_borderWidth;

//     bool handleResizeHitTest(MSG* msg, long long* result) {
//         QPoint pos = QCursor::pos();
//         QPoint localPos = pos - m_targetWidget->geometry().topLeft();
//         int w = m_targetWidget->width();
//         int h = m_targetWidget->height();

//         bool lx = localPos.x() < m_borderWidth;
//         bool rx = localPos.x() > w - m_borderWidth;
//         bool ty = localPos.y() < m_borderWidth;
//         bool by = localPos.y() > h - m_borderWidth;

//         if (lx && ty)       { *result = HTTOPLEFT; }
//         else if (rx && by)  { *result = HTBOTTOMRIGHT; }
//         else if (rx && ty)  { *result = HTTOPRIGHT; }
//         else if (lx && by)  { *result = HTBOTTOMLEFT; }
//         else if (ty)        { *result = HTTOP; }
//         else if (by)        { *result = HTBOTTOM; }
//         else if (lx)        { *result = HTLEFT; }
//         else if (rx)        { *result = HTRIGHT; }
//         else return false;

//         return true;
//     }

//     bool handleTitleBarButtons(MSG* msg) {
//         // This is a simplified version. You'd need to adapt this to your specific title bar implementation
//         switch(msg->message) {
//         case WM_NCHITTEST: {
//             // Example of handling max button hover state
//             // Requires custom implementation of your title bar logic
//             break;
//         }
//         case WM_MOUSELEAVE: {
//             // Reset button state
//             break;
//         }
//         case WM_NCLBUTTONDOWN:
//         case WM_NCLBUTTONDBLCLK: {
//             // Handle button click
//             break;
//         }
//         case WM_NCLBUTTONUP:
//         case WM_NCRBUTTONUP: {
//             // Handle button release
//             break;
//         }
//         }
//         return false;
//     }

//     bool handleNCCalcSize(MSG* msg, long long* result) {
//         NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);

//         // Check if window is maximized
//         bool isMaximized = IsZoomed(msg->hwnd);

//         if (msg->wParam == TRUE && isMaximized) {
//             // Adjust client rect for maximized window
//             int borderThickness = GetSystemMetrics(SM_CXSIZEFRAME);

//             params->rgrc[0].left += borderThickness;
//             params->rgrc[0].top += borderThickness;
//             params->rgrc[0].right -= borderThickness;
//             params->rgrc[0].bottom -= borderThickness;
//         }

//         *result = 0;
//         return true;
//     }
// };


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

    enum ResizeEdge {
        NoEdge = 0,
        LeftEdge = 2,
        TopEdge = 1,
        RightEdge = 4,
        BottomEdge = 8
    };

    bool _mousePressed = false, _globalDrag = false, _curDragging = false, _systemResizeStarted = false;
    int _resizeEdges = NoEdge, _resizeAreaMargin = 10, _titleBarHeight = 30;
    QPointF _lastMousePos, _lastWindowPosition;
    QRect _startGeometry;

    std::unique_ptr<CustomQtEventFilter> _eventFilter;
};

CustomQQuickWindow::CustomQQuickWindow(QWindow *parent)
    : QQuickWindow(parent) {
    setFlags(Qt::FramelessWindowHint | Qt::Window);

    _p = new CustomQQuickWindowPriv;

#ifdef Q_OS_WIN
    HWND hwnd = (HWND)winId();

    BOOL v = TRUE;
    DwmSetWindowAttribute((HWND)winId(), DWMWA_USE_IMMERSIVE_DARK_MODE, &v, sizeof(v));
#endif

    connect(this, &CustomQQuickWindow::sceneGraphInitialized, this, [hwnd, this] {

#ifdef Q_OS_WIN
        DWORD t = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &t, sizeof(t));

        DWORD value = DWMSBT_MAINWINDOW;
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));
#endif

        QRect geom = QSettings().value("CustomQQuickWindow/geometry").toRect();
        if ( geom.width() > 10 && geom.height() > 10 )
            this->setGeometry(geom);
    });

    connect(this, &QQuickWindow::visibilityChanged, this, [this](QWindow::Visibility visibility){
        if ( visibility == QWindow::Maximized ) {
#ifdef Q_OS_WIN
            DWORD t = 0;
            DwmSetWindowAttribute((HWND)winId(), DWMWA_WINDOW_CORNER_PREFERENCE, &t, sizeof(t));
#endif
        } else if ( visibility == QWindow::Windowed ) {
#ifdef Q_OS_WIN
            DWORD t = DWMWCP_ROUND;
            DwmSetWindowAttribute((HWND)winId(), DWMWA_WINDOW_CORNER_PREFERENCE, &t, sizeof(t));
#endif
        }
    });

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

void CustomQQuickWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        _p->_mousePressed = true;
        _p->_lastMousePos = event->globalPosition();
        _p->_startGeometry = geometry();
        _p->_lastWindowPosition = position();

        if ( ( _p->_lastMousePos.y() <= geometry().top() + ( _p->_titleBarHeight + _p->_resizeAreaMargin ) )
            && ( _p->_lastMousePos.y() >= geometry().top() + _p->_resizeAreaMargin ) ) {
            _p->_curDragging = true;
        }
    }
}

void CustomQQuickWindow::mouseMoveEvent(QMouseEvent *event) {
    if (_p->_mousePressed) {
        QPointF delta = event->globalPosition() - _p->_lastMousePos;
        QRect newGeometry = _p->_startGeometry;

        // Global Drag
        if ( _p->_globalDrag && _p->_resizeEdges == _p->NoEdge ) {
            startSystemMove();
            return;
        }

        if ( _p->_curDragging && _p->_resizeEdges == _p->NoEdge ) {
            startSystemMove();
            return;
        }

        if ( _p->_systemResizeStarted )
            return;

        Qt::Edges resizeFlags;
        if (_p->_resizeEdges & _p->TopEdge)
            resizeFlags = resizeFlags | Qt::TopEdge;
        if (_p->_resizeEdges & _p->BottomEdge)
            resizeFlags = resizeFlags | Qt::BottomEdge;
        if (_p->_resizeEdges & _p->LeftEdge)
            resizeFlags = resizeFlags | Qt::LeftEdge;
        if (_p->_resizeEdges & _p->RightEdge)
            resizeFlags = resizeFlags | Qt::RightEdge;

        if ( resizeFlags ) {
            startSystemResize(resizeFlags);
            _p->_systemResizeStarted = true;
        }

    } else {
        updateCursorShape(event->pos());
    }
}

void CustomQQuickWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        _p->_mousePressed = false;
        _p->_curDragging = false;
        _p->_systemResizeStarted = false;
        _p->_resizeEdges = _p->NoEdge;

        setCursor(Qt::ArrowCursor);
    }
}

void CustomQQuickWindow::updateCursorShape(const QPoint &pos) {
    QRect rect = this->geometry();

    QPoint globalPos = mapToGlobal(pos);

    _p->_resizeEdges = _p->NoEdge;
    if (globalPos.y() <= rect.top() + _p->_resizeAreaMargin)
        _p->_resizeEdges |= _p->TopEdge;
    if (globalPos.y() >= rect.bottom() - _p->_resizeAreaMargin)
        _p->_resizeEdges |= _p->BottomEdge;
    if (globalPos.x() <= rect.left() + _p->_resizeAreaMargin)
        _p->_resizeEdges |= _p->LeftEdge;
    if (globalPos.x() >= rect.right() - _p->_resizeAreaMargin)
        _p->_resizeEdges |= _p->RightEdge;

    if (_p->_resizeEdges == (_p->TopEdge | _p->LeftEdge) || _p->_resizeEdges == (_p->BottomEdge | _p->RightEdge))
        setCursor(Qt::SizeFDiagCursor);
    else if (_p->_resizeEdges == (_p->TopEdge | _p->RightEdge) || _p->_resizeEdges == (_p->BottomEdge | _p->LeftEdge))
        setCursor(Qt::SizeBDiagCursor);
    else if (_p->_resizeEdges & (_p->TopEdge | _p->BottomEdge))
        setCursor(Qt::SizeVerCursor);
    else if (_p->_resizeEdges & (_p->LeftEdge | _p->RightEdge))
        setCursor(Qt::SizeHorCursor);
    else
        setCursor(Qt::ArrowCursor);

}
