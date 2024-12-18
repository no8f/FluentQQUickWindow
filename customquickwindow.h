//-------------------------------------------------------
//
//           SPDX-License-Identifier: LGPLv2
//
//             Copyright © 2024 Noah Felber
//
//      All Rights Reserved/Alle Rechte vorbehalten.
//
//-------------------------------------------------------

#pragma once

#include <QQuickWindow>
#include <QMouseEvent>
#include <QCursor>

class CustomQQuickWindowPriv;
/*!
 * \brief The CustomQQuickWindow class
 *
 * Provides a superset which is also a downgrade of the original QQuickWindow,
 * with the support for a system drawn backdrop, a custom Title bar and global dragging
 */
class CustomQQuickWindow : public QQuickWindow {
    Q_OBJECT
    QML_ELEMENT

public:
    CustomQQuickWindow(QWindow* parent = nullptr);

    /*!
     * \brief Enable or disable global window dragging
     * \param enable: wether the global dragging should be enabled
     */
    Q_INVOKABLE const void setGlobalDrag(const bool enable = false);

    /*!
     * \brief Get if the global dragging is enabled
     * \return bool: global dragging enabled
     */
    Q_INVOKABLE const bool globalDrag();

    /*!
     * \brief Sets the width of the area at the edge of the window where resizing is activated
     * \param margin: The width
     */
    Q_INVOKABLE const void setResizeMargin(const int margin = 10);

    /*!
     * \brief retrieve the width of the resize margin
     * \return int: The margin
     */
    Q_INVOKABLE const int resizeMargin();

    /*!
     * \brief Sets the titleBar height
     * \param height: The height
     *
     * This is the area at the Top where you can move the window whon global draggin is disabled.
     * It also displays the window title and Buttons
     */
    Q_INVOKABLE const void setTitleBarHeight(const int height = 30);

    /*!
     * \brief retrieve the current titlebar height
     * \return int: the current titlebar height
     */
    Q_INVOKABLE const int titleBarHeight();

    /*!
     * \brief Call before loading your QML Engine
     * Resgisters the component
     */
    static const void registerQmlType();

signals:
    void resized(const QRect& newSize);

protected:
    void mousePressEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;

    void updateCursorShape(const QPoint& pos);

private:
    CustomQQuickWindowPriv* _p;
};
