import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef

/**
 * FloatingToolbar.qml - плавающая панель инструментов.
 */
Rectangle {
    id: root
    
    required property BoardController controller
    
    signal settingsClicked()
    
    width: 58
    height: toolbarLayout.height + 16
    color: Qt.rgba(31/255, 31/255, 31/255, 0.9)
    border.color: Theme.borderColor
    border.width: 3
    radius: 12
    
    ColumnLayout {
        id: toolbarLayout
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 8
        spacing: 5
        
        // Файловые операции
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/paste.svg"
            tooltip: "Открыть"
            shortcutText: "Ctrl+O"
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/paste.svg"
            tooltip: "Сохранить"
            shortcutText: "Ctrl+S"
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/paste.svg"
            tooltip: "Сохранить как"
            shortcutText: "Ctrl+Shift+S"
        }
        
        ToolbarSeparator {}
        
        // Буфер обмена
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/paste.svg"
            tooltip: "Вставить"
            shortcutText: "Ctrl+V"
            onClicked: controller.pasteFromClipboard(0, 0)
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/delete.svg"
            tooltip: "Удалить"
            shortcutText: "Delete"
            onClicked: controller.deleteSelected()
        }
        
        ToolbarSeparator {}
        
        // Трансформации
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/grid.svg"
            tooltip: "Привязать к сетке"
            shortcutText: "Ctrl+G"
            onClicked: controller.snapToGrid()
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/resize.svg"
            tooltip: "Изменить размер"
            shortcutText: "Ctrl+E"
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/rotate-left.svg"
            tooltip: "Вращать против часовой"
            shortcutText: "Ctrl+Shift+R"
            onClicked: controller.rotateSelected(-90)
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/rotate-right.svg"
            tooltip: "Вращать по часовой"
            shortcutText: "Ctrl+R"
            onClicked: controller.rotateSelected(90)
        }
        
        ToolbarSeparator {}
        
        // Масштаб
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/zoom-in.svg"
            tooltip: "Приблизить"
            shortcutText: "Ctrl++"
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/zoom-out.svg"
            tooltip: "Отдалить"
            shortcutText: "Ctrl+-"
        }
        
        ToolbarSeparator {}
        
        // Undo/Redo
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/undo.svg"
            tooltip: "Отменить"
            shortcutText: "Ctrl+Z"
            enabled: controller.canUndo
            onClicked: controller.undo()
        }
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/redo.svg"
            tooltip: "Повторить"
            shortcutText: "Ctrl+Shift+Z"
            enabled: controller.canRedo
            onClicked: controller.redo()
        }
        
        ToolbarSeparator {}
        
        // Настройки
        ImagoToolButton {
            iconSource: "qrc:/icons/icons/settings.svg"
            tooltip: "Настройки"
            shortcutText: "Ctrl+,"
            onClicked: root.settingsClicked()
        }
    }
    
    // Разделитель
    component ToolbarSeparator: Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 1
        Layout.leftMargin: 5
        Layout.rightMargin: 5
        color: Theme.borderColor
    }
}
