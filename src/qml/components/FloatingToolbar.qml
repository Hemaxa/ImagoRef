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
            iconSource: ThemeManager.pasteIconPath
            tooltip: "Вставить"
            shortcutText: "Ctrl+V"
            onClicked: root.pasteClicked()
        }
        
        // Удалить
        ToolbarButton {
            iconSource: ThemeManager.deleteIconPath
            tooltip: "Удалить"
            shortcutText: "Delete"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.deleteSelected()
        }
        
        // Привязать к сетке
        ToolbarButton {
            iconSource: ThemeManager.gridSnapIconPath
            tooltip: "Привязать к сетке"
            shortcutText: "G"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.snapToGrid()
        }
        
        // Увеличить разрешение
        ToolbarButton {
            iconSource: ThemeManager.upscaleIconPath
            tooltip: "Увеличить разрешение"
            shortcutText: "U"
            visible: ModelsManager.isModelDownloaded
            enabled: controller.selectionController.hasSelection
            onClicked: {
                var selectedIndices = controller.model.getSelectedIndices()
                for (var i = 0; i < selectedIndices.length; ++i) {
                    controller.upscaleController.upscaleImage(selectedIndices[i])
                }
            }
        }
        
        // Изменить размер
        ToolbarButton {
            iconSource: ThemeManager.scaleIconPath
            tooltip: "Изменить размер"
            shortcutText: "S"
            active: root.resizeModeActive
            enabled: controller.selectionController.hasSelection
            onClicked: root.resizeModeClicked()
        }
        
        // Обрезать
        ToolbarButton {
            iconSource: ThemeManager.cropIconPath
            tooltip: "Обрезать"
            shortcutText: "C"
            active: root.cropModeActive
            enabled: controller.selectionController.hasSelection
            onClicked: root.cropModeClicked()
        }
        
        // Подписать
        ToolbarButton {
            iconSource: ThemeManager.labelIconPath
            tooltip: "Подписать"
            shortcutText: "L"
            enabled: controller.selectionController.hasSelection
            onClicked: root.labelClicked()
        }
        
        // Расположить
        ToolbarButton {
            iconSource: ThemeManager.arrangeIconPath
            tooltip: "Расположить"
            shortcutText: "A"
            onClicked: controller.toolController.arrangeAll()
        }
        
        // Вращать против часовой
        ToolbarButton {
            iconSource: ThemeManager.rotateLeftIconPath
            tooltip: "Вращать против часовой"
            shortcutText: "Shift+R"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.rotateSelected(-90)
        }
        
        // Вращать по часовой
        ToolbarButton {
            iconSource: ThemeManager.rotateRightIconPath
            tooltip: "Вращать по часовой"
            shortcutText: "R"
            enabled: controller.selectionController.hasSelection
            onClicked: controller.toolController.rotateSelected(90)
        }
        
        // Приблизить
        ToolbarButton {
            iconSource: ThemeManager.zoomInIconPath
            tooltip: "Приблизить"
            shortcutText: "Ctrl++"
            onClicked: root.zoomInClicked()
        }
        
        // Отдалить
        ToolbarButton {
            iconSource: ThemeManager.zoomOutIconPath
            tooltip: "Отдалить"
            shortcutText: "Ctrl+-"
            onClicked: root.zoomOutClicked()
        }
        
        // Отменить
        ToolbarButton {
            iconSource: ThemeManager.undoIconPath
            tooltip: "Отменить"
            shortcutText: "Ctrl+Z"
            enabled: controller.canUndo
            onClicked: controller.undo()
        }
        
        // Повторить
        ToolbarButton {
            iconSource: ThemeManager.redoIconPath
            tooltip: "Повторить"
            shortcutText: "Ctrl+Shift+Z"
            enabled: controller.canRedo
            onClicked: controller.redo()
        }

        // Закрепить поверх окон
        ToolbarButton {
            iconSource: ThemeManager.pinIconPath
            tooltip: "Закрепить поверх всех окон"
            active: controller.toolController.isPinned
            onClicked: controller.toolController.togglePin()
        }
        
        // Настройки
        ToolbarButton {
            iconSource: ThemeManager.settingsIconPath
            tooltip: "Настройки"
            shortcutText: "Ctrl+,"
            onClicked: root.settingsClicked()
        }
    }
}
