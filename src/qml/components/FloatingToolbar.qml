//FloatingToolbar.qml - плавающая панель инструментов

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef

Rectangle {
    id: root
    
    required property BoardController controller
    
    //cигналы для действий
    signal settingsClicked()
    signal zoomInClicked()
    signal zoomOutClicked()
    signal resizeModeClicked()
    signal cropModeClicked()
    signal labelClicked()
    signal pasteClicked()
    
    //cостояние активных инструментов
    property bool resizeModeActive: false
    property bool cropModeActive: false
    
    width: 60
    height: toolbarLayout.height + 6
    color: ThemeManager.panelColor
    border.color: ThemeManager.borderColor
    border.width: 3
    radius: 3
    clip: true  // Обрезаем содержимое по скруглённым краям
    
    ColumnLayout {
        id: toolbarLayout
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 3
        spacing: 3
        
        // Вставить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/paste.svg"
            tooltip: "Вставить"
            shortcutText: "Ctrl+V"
            onClicked: root.pasteClicked()
        }
        
        // Удалить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/delete.svg"
            tooltip: "Удалить"
            shortcutText: "Delete"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.deleteSelected()
        }
        
        // Привязать к сетке
        ToolbarButton {
            iconSource: "qrc:/icons/icons/grid_snap.svg"
            tooltip: "Привязать к сетке"
            shortcutText: "G"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.snapToGrid()
        }
        
        // Изменить размер
        ToolbarButton {
            iconSource: "qrc:/icons/icons/scale.svg"
            tooltip: "Изменить размер"
            shortcutText: "S"
            active: root.resizeModeActive
            enabled: controller.selectionController.hasSelection
            onClicked: root.resizeModeClicked()
        }
        
        // Обрезать
        ToolbarButton {
            iconSource: "qrc:/icons/icons/crop.svg"
            tooltip: "Обрезать"
            shortcutText: "C"
            active: root.cropModeActive
            enabled: controller.selectionController.hasSelection
            onClicked: root.cropModeClicked()
        }
        
        // Подписать
        ToolbarButton {
            iconSource: "qrc:/icons/icons/label.svg"
            tooltip: "Подписать"
            shortcutText: "L"
            enabled: controller.selectionController.hasSelection
            onClicked: root.labelClicked()
        }
        
        // Расположить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/arrange.svg"
            tooltip: "Расположить"
            shortcutText: "A"
            onClicked: controller.toolController.arrangeAll()
        }
        
        // Вращать против часовой
        ToolbarButton {
            iconSource: "qrc:/icons/icons/rotate_left.svg"
            tooltip: "Вращать против часовой"
            shortcutText: "Shift+R"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.rotateSelected(-90)
        }
        
        // Вращать по часовой
        ToolbarButton {
            iconSource: "qrc:/icons/icons/rotate_right.svg"
            tooltip: "Вращать по часовой"
            shortcutText: "R"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.rotateSelected(90)
        }
        
        // Приблизить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/zoom_in.svg"
            tooltip: "Приблизить"
            shortcutText: "Ctrl++"
            onClicked: root.zoomInClicked()
        }
        
        // Отдалить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/zoom_out.svg"
            tooltip: "Отдалить"
            shortcutText: "Ctrl+-"
            onClicked: root.zoomOutClicked()
        }
        
        // Отменить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/undo.svg"
            tooltip: "Отменить"
            shortcutText: "Ctrl+Z"
            enabled: controller.canUndo
            onClicked: controller.undo()
        }
        
        // Повторить
        ToolbarButton {
            iconSource: "qrc:/icons/icons/redo.svg"
            tooltip: "Повторить"
            shortcutText: "Ctrl+Shift+Z"
            enabled: controller.canRedo
            onClicked: controller.redo()
        }
        
        // Настройки
        ToolbarButton {
            iconSource: "qrc:/icons/icons/settings.svg"
            tooltip: "Настройки"
            shortcutText: "Ctrl+,"
            onClicked: root.settingsClicked()
        }
    }
}
