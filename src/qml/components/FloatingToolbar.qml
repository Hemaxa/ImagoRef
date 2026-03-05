import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef

/**
 * FloatingToolbar.qml - плавающая панель инструментов.
 * Тёмно-серая панель со скруглёнными краями, иконки на всю ширину.
 */
Rectangle {
    id: root
    
    required property BoardController controller
    
    // Сигналы для действий
    signal settingsClicked()
    signal zoomInClicked()
    signal zoomOutClicked()
    signal resizeModeClicked()
    signal cropModeClicked()
    signal labelClicked()
    signal pasteClicked()
    
    // Состояние активных инструментов
    property bool resizeModeActive: false
    property bool cropModeActive: false
    
    width: 58
    height: toolbarLayout.height + 6
    color: ThemeManager.panelColor
    border.color: ThemeManager.borderColor
    border.width: 3
    radius: 12
    clip: true  // Обрезаем содержимое по скруглённым краям
    
    ColumnLayout {
        id: toolbarLayout
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 3
        spacing: 3
        
        // Вставить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/paste.svg"
            tooltip: "Вставить"
            shortcutText: "Ctrl+V"
            onClicked: root.pasteClicked()
        }
        
        // Удалить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/delete.svg"
            tooltip: "Удалить"
            shortcutText: "Delete"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.deleteSelected()
        }
        
        // Привязать к сетке
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/grid_snap.svg"
            tooltip: "Привязать к сетке"
            shortcutText: "G"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.snapToGrid()
        }
        
        // Изменить размер
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/scale.svg"
            tooltip: "Изменить размер"
            shortcutText: "S"
            active: root.resizeModeActive
            enabled: controller.selectionController.hasSelection
            onClicked: root.resizeModeClicked()
        }
        
        // Обрезать
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/crop.svg"
            tooltip: "Обрезать"
            shortcutText: "C"
            active: root.cropModeActive
            enabled: controller.selectionController.hasSelection
            onClicked: root.cropModeClicked()
        }
        
        // Подписать
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/label.svg"
            tooltip: "Подписать"
            shortcutText: "L"
            enabled: controller.selectionController.hasSelection
            onClicked: root.labelClicked()
        }
        
        // Расположить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/arrange.svg"
            tooltip: "Расположить"
            shortcutText: "A"
            onClicked: controller.toolController.arrangeAll()
        }
        
        // Вращать против часовой
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/rotate_left.svg"
            tooltip: "Вращать против часовой"
            shortcutText: "Shift+R"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.rotateSelected(-90)
        }
        
        // Вращать по часовой
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/rotate_right.svg"
            tooltip: "Вращать по часовой"
            shortcutText: "R"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.rotateSelected(90)
        }
        
        // Приблизить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/zoom_in.svg"
            tooltip: "Приблизить"
            shortcutText: "Ctrl++"
            onClicked: root.zoomInClicked()
        }
        
        // Отдалить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/zoom_out.svg"
            tooltip: "Отдалить"
            shortcutText: "Ctrl+-"
            onClicked: root.zoomOutClicked()
        }
        
        // Отменить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/undo.svg"
            tooltip: "Отменить"
            shortcutText: "Ctrl+Z"
            enabled: controller.canUndo
            onClicked: controller.undo()
        }
        
        // Повторить
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/redo.svg"
            tooltip: "Повторить"
            shortcutText: "Ctrl+Shift+Z"
            enabled: controller.canRedo
            onClicked: controller.redo()
        }
        
        // Настройки
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/settings.svg"
            tooltip: "Настройки"
            shortcutText: "Ctrl+,"
            onClicked: root.settingsClicked()
        }
    }
}
