import QtQuick
import QtQuick.Controls
import ImagoRef

// Невидимый слой-оверлей для захвата курсора мыши при работе пипетки
Item {
    id: root
    
    // Получаем контроллер
    required property BoardController controller
    
    // Видимость привязана к свойству в ToolController
    visible: controller.toolController.isEyedropperActive
    anchors.fill: parent
    z: 9999 // Поверх всего

    property color currentColor: "#000000"
    property string colorText: "#000000"

    // Таймер для тротлинга захвата цвета экрана
    Timer {
        id: colorUpdateTimer
        interval: 16 // ~60fps
        repeat: false
        property int lastGlobalX: 0
        property int lastGlobalY: 0
        onTriggered: {
            currentColor = controller.toolController.getColorAtPoint(lastGlobalX, lastGlobalY)
            var mode = SettingsManager.colorCopyMode
            if (mode === 0) { // HEX
                colorText = currentColor.toString().toUpperCase()
            } else { // RGB
                colorText = "RGB(" + Math.round(currentColor.r * 255) + ", " + 
                            Math.round(currentColor.g * 255) + ", " + 
                            Math.round(currentColor.b * 255) + ")"
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.CrossCursor //Крестообразный курсор
        
        onPositionChanged: (mouse) => {
            // Конвертируем локальные координаты в глобальные экранные
            var globalPos = root.mapToGlobal(mouse.x, mouse.y)
            colorUpdateTimer.lastGlobalX = globalPos.x
            colorUpdateTimer.lastGlobalY = globalPos.y
            colorUpdateTimer.restart()
            
            // Двигаем виджет пипетки за курсором
            // Центрируем с отступом чуть левее и выше, чтобы курсор не перекрывал
            magnifier.x = mouse.x - magnifier.width / 2
            magnifier.y = mouse.y - magnifier.height - 20
        }
        
        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
                // Копируем цвет и отключаем инструмент
                controller.toolController.copyColorToClipboard(currentColor)
                controller.toolController.toggleEyedropper()
            } else if (mouse.button === Qt.RightButton) {
                // Правая кнопка - отмена
                controller.toolController.toggleEyedropper()
            }
        }
    }

    // Кружок пипетки (цвет)
    Item {
        id: magnifier
        width: 100
        height: 100
        visible: mouseArea.containsMouse

        // Внутренний круг цветом
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            width: 80
            height: 80
            radius: 40
            color: root.currentColor
            border.color: ThemeManager.colors.borderColor
            border.width: 3
            
            // Центральный маркер
            Rectangle {
                anchors.centerIn: parent
                width: 6
                height: 6
                radius: 3
                color: "transparent"
                border.color: "white"
                border.width: 1
                
                Rectangle {
                    anchors.centerIn: parent
                    width: 2
                    height: 2
                    color: "black"
                }
            }
        }
        
        // Подпись цвета
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.top
            anchors.bottomMargin: 5
            width: colorLabel.implicitWidth + 24
            height: 28
            color: Qt.rgba(ThemeManager.colors.panelColor.r, ThemeManager.colors.panelColor.g, ThemeManager.colors.panelColor.b, 0.9)
            border.color: ThemeManager.colors.borderColor
            border.width: 1
            radius: 14
            
            Text {
                id: colorLabel
                anchors.centerIn: parent
                text: root.colorText
                color: ThemeManager.colors.textColor
                font.pixelSize: 12
                font.bold: true
            }
        }
    }
}
