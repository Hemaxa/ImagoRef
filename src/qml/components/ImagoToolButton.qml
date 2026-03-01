import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * ImagoToolButton.qml - кнопка панели инструментов.
 * Иконка отображается на всю ширину кнопки.
 * Новые SVG-иконки содержат встроенный цветной фон.
 */
AbstractButton {
    id: root
    
    property string iconSource
    property string tooltip
    property string shortcutText
    property bool active: false
    
    implicitWidth: 52
    implicitHeight: 52
    
    opacity: enabled ? 1.0 : 0.4
    
    // Масштаб для анимации
    property real iconScale: 1.0
    
    background: Item {}
    
    contentItem: Item {
        // Контейнер для закругления
        Rectangle {
            id: clipRect
            anchors.fill: parent
            radius: 6
            clip: true
            color: "transparent"
            
            Image {
                id: iconImage
                anchors.centerIn: parent
                width: parent.width * root.iconScale
                height: parent.height * root.iconScale
                source: root.iconSource
                sourceSize: Qt.size(100, 100)
                smooth: true
                fillMode: Image.PreserveAspectFit
            }
        }
        
        // Обводка для active состояния
        Rectangle {
            anchors.fill: parent
            radius: 6
            color: "transparent"
            border.color: root.active ? Theme.accentColor : "transparent"
            border.width: root.active ? 2 : 0
        }
        
        // Затемнение при нажатии
        Rectangle {
            anchors.fill: parent
            radius: 6
            color: root.pressed ? Qt.rgba(0, 0, 0, 0.25) : "transparent"
        }
    }
    
    // Анимация масштаба при наведении
    states: [
        State {
            name: "hovered"
            when: root.hovered && !root.pressed
            PropertyChanges {
                target: root
                iconScale: 1.08
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
