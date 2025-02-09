import QtQuick.Controls

import QtQuick.Controls.FluentWinUI3

import FluentQQUickWindow 1.0

CustomQQuickWindow {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    title: "Test"
    color: "transparent"

    Switch {
        anchors.top: parent.top
    }
}
