import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef

// Панель истории цветов
Rectangle {
    id: root
    
    required property BoardController controller

    // История цветов берется из настроек
    property var colorHistory: SettingsManager.colorHistory
    
    // Скрыто по умолчанию
    property bool isPanelVisible: false

    width: layout.implicitWidth + 30
    height: 50

    color: Qt.rgba(ThemeManager.colors.panelColor.r, ThemeManager.colors.panelColor.g, ThemeManager.colors.panelColor.b, 1.0)
    border.color: ThemeManager.colors.borderColor
    border.width: 1
    radius: 25

    // Позиционируем снизу по центру
    x: (parent.width - width) / 2
    y: isPanelVisible ? parent.height - height - 20 : parent.height + 20

    // Скрываем если панель за границами
    visible: y < parent.height

    Behavior on y {
        NumberAnimation {
            duration: 300
            easing.type: Easing.InOutCubic
        }
    }

    RowLayout {
        id: layout
        anchors.centerIn: parent
        spacing: 12

        Repeater {
            model: root.colorHistory
            
            delegate: Rectangle {
                width: 30
                height: 30
                radius: 15
                color: modelData
                border.color: ThemeManager.colors.borderColor
                border.width: 1
                
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    
                    onClicked: {
                        controller.toolController.copyColorToClipboard(modelData)
                    }
                    
                    ToolTip.visible: containsMouse
                    ToolTip.text: "Скопировать " + modelData
                    ToolTip.delay: 400
                }
            }
        }

        // Заглушка, если история пуста
        Text {
            visible: root.colorHistory.length === 0
            text: "История пипетки"
            color: ThemeManager.colors.textColor
            font.pixelSize: 13
            opacity: 0.6
        }
    }
}
