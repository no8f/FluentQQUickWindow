import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts

import QtQuick.Controls.FluentWinUI3

import FluentQQUickWindow 1.0

CustomQQuickWindow {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    title: "Test"
    color: "transparent"

    RowLayout {
        id: titleBtnRow
        anchors.top: parent.top
        anchors.right: parent.right
        uniformCellSizes: true
        spacing: 0
        width: 150
        height: 30

        Button {
            flat: true
            icon.source: "qrc:/icons/ic_fluent_chevron_down_24_filled.svg"
            Layout.maximumWidth: titleBtnRow.width / 3
            icon.height: 12
            icon.width: 12
            onClicked: {
                mainWindow.showMinimized()
            }
        }
        Button {
            flat: true
            icon.source: "qrc:/icons/ic_fluent_maximize_24_filled.svg"
            Layout.maximumWidth: titleBtnRow.width / 3
            icon.height: 12
            icon.width: 12
            onClicked: {
                if ( mainWindow.visibility == 4 )
                    mainWindow.showNormal()
                else
                    mainWindow.showMaximized()
            }
        }
        Button {
            flat: true
            icon.source: "qrc:/icons/ic_fluent_dismiss_24_filled.svg"
            Layout.maximumWidth: titleBtnRow.width / 3
            icon.height: 12
            icon.width: 12
            onClicked: {
                mainWindow.close()
            }
        }

    }

    Switch {
        anchors.centerIn: parent
    }
}
