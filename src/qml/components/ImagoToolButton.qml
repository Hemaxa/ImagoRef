import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * ToolButton.qml - анимированная кнопка для панели инструментов.
 * Поддерживает SVG иконки и анимацию масштаба.
 */
AbstractButton {
    id: root
    
    property string iconSource
    property string tooltip
    property string shortcutText
    
    implicitWidth: 38
    implicitHeight: 38
    
    opacity: enabled ? 1.0 : 0.4
    
    // Масштаб иконки для анимации
    property real iconScale: 1.0
    
    background: Rectangle {
        radius: 6
        color: root.pressed ? Theme.accentPressedColor : 
               root.hovered ? Theme.accentHoverColor : "transparent"
    }
    
    contentItem: Item {
        Image {
            id: iconImage
            anchors.centerIn: parent
            width: 22 * root.iconScale
            height: 22 * root.iconScale
            source: root.iconSource
            sourceSize: Qt.size(22, 22)
            smooth: true
        }
    }
    
    // Анимация масштаба при наведении
    states: [
        State {
            name: "hovered"
            when: root.hovered && !root.pressed
            PropertyChanges {
                target: root
                iconScale: 1.15
            }
        }
    ]
    
    transitions: Transition {
        NumberAnimation {
            property: "iconScale"
            duration: 100
            easing.type: Easing.OutCubic
        }
    }
    
    // Всплывающая подсказка
    ToolTip.visible: hovered && tooltip
    ToolTip.text: tooltip + (shortcutText ? " (" + shortcutText + ")" : "")
    ToolTip.delay: 500
}
