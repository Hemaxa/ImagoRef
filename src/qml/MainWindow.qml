import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import ImagoRef
import "components"

/**
 * MainWindow.qml - главное окно приложения с холстом и панелью инструментов.
 */
Item {
    id: root
    
    required property BoardController controller
    signal settingsRequested()
    
    // Холст
    CanvasView {
        id: canvasView
        anchors.fill: parent
        controller: root.controller
        z: 0
    }
    
    // Плавающая панель инструментов
    FloatingToolbar {
        id: toolbar
        controller: root.controller
        z: 100
        
        x: 15
        y: 15
        
        // Подключаем все сигналы
        onSettingsClicked: root.settingsRequested()
        onOpenClicked: fileOpenDialog.open()
        onSaveClicked: {
            if (controller.currentFilePath !== "") {
                controller.saveBoard()
            } else {
                fileSaveDialog.open()
            }
        }
        onSaveAsClicked: fileSaveDialog.open()
        onZoomInClicked: canvasView.zoomIn()
        onZoomOutClicked: canvasView.zoomOut()
        onResizeModeClicked: canvasView.toggleResizeMode()
        
        // Состояние видимости
        state: "visible"
        states: [
            State {
                name: "visible"
                PropertyChanges { target: toolbar; x: 15 }
            },
            State {
                name: "hidden"
                PropertyChanges { target: toolbar; x: -toolbar.width - 10 }
            }
        ]
        
        transitions: Transition {
            NumberAnimation { 
                property: "x"
                duration: 300
                easing.type: Easing.InOutCubic
            }
        }
    }
    
    // Горячие клавиши
    
    // Tab - показать/скрыть панель
    Shortcut {
        sequence: "Tab"
        onActivated: toolbar.state = (toolbar.state === "visible") ? "hidden" : "visible"
    }
    
    // Delete - удалить выделенные
    Shortcut {
        sequences: ["Delete", "Backspace"]
        onActivated: controller.deleteSelected()
    }
    
    // Paste - вставить из буфера
    Shortcut {
        sequence: StandardKey.Paste
        onActivated: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.pasteFromClipboard(scenePos.x, scenePos.y)
        }
    }
    
    // Open
    Shortcut {
        sequence: StandardKey.Open
        onActivated: fileOpenDialog.open()
    }
    
    // Save
    Shortcut {
        sequence: StandardKey.Save
        onActivated: {
            if (controller.currentFilePath !== "") {
                controller.saveBoard()
            } else {
                fileSaveDialog.open()
            }
        }
    }
    
    // Save As
    Shortcut {
        sequence: StandardKey.SaveAs
        onActivated: fileSaveDialog.open()
    }
    
    // Snap to grid
    Shortcut {
        sequence: "Ctrl+G"
        onActivated: controller.snapToGrid()
    }
    
    // Rotate
    Shortcut {
        sequence: "Ctrl+R"
        onActivated: controller.rotateSelected(90)
    }
    
    Shortcut {
        sequence: "Ctrl+Shift+R"
        onActivated: controller.rotateSelected(-90)
    }
    
    // Undo/Redo
    Shortcut {
        sequence: StandardKey.Undo
        onActivated: controller.undo()
    }
    
    Shortcut {
        sequence: StandardKey.Redo
        onActivated: controller.redo()
    }
    
    // Zoom
    Shortcut {
        sequences: ["Ctrl+=", "Ctrl++"]
        onActivated: canvasView.zoomIn()
    }
    
    Shortcut {
        sequences: ["Ctrl+-"]
        onActivated: canvasView.zoomOut()
    }
    
    // Escape - сбросить выделение и режим ресайза
    Shortcut {
        sequence: "Escape"
        onActivated: {
            controller.clearSelection()
            canvasView.exitResizeMode()
        }
    }
    
    // Resize mode
    Shortcut {
        sequence: "Ctrl+E"
        onActivated: canvasView.toggleResizeMode()
    }
    
    // Select all
    Shortcut {
        sequence: StandardKey.SelectAll
        onActivated: controller.selectAll()
    }
    
    // Диалог сохранения
    FileDialog {
        id: fileSaveDialog
        title: "Сохранить доску"
        fileMode: FileDialog.SaveFile
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: controller.saveBoardAs(selectedFile)
    }
    
    // Диалог открытия
    FileDialog {
        id: fileOpenDialog
        title: "Открыть доску"
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: controller.openBoard(selectedFile)
    }
}
