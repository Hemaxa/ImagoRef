import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef
import "../components"

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
        
        // Подключаем сигналы
        onSettingsClicked: root.settingsRequested()
        onZoomInClicked: canvasView.zoomIn()
        onZoomOutClicked: canvasView.zoomOut()
        onResizeModeClicked: canvasView.toggleResizeMode()
        onCropModeClicked: canvasView.toggleCropMode()
        onLabelClicked: labelDialog.openDialog()
        onPasteClicked: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.pasteFromClipboard(scenePos.x, scenePos.y)
        }
        
        // Привязка состояния активных инструментов
        resizeModeActive: canvasView.resizeMode
        cropModeActive: canvasView.cropMode
        
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
    
    // Paste - вставить из буфера (Ctrl/Cmd+V)
    Shortcut {
        sequence: StandardKey.Paste
        onActivated: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.pasteFromClipboard(scenePos.x, scenePos.y)
        }
    }
    
    // Snap to grid — G
    Shortcut {
        sequence: "G"
        onActivated: controller.snapToGrid()
    }
    
    // Rotate clockwise — R
    Shortcut {
        sequence: "R"
        onActivated: controller.rotateSelected(90)
    }
    
    // Rotate counter-clockwise — Shift+R
    Shortcut {
        sequence: "Shift+R"
        onActivated: controller.rotateSelected(-90)
    }
    
    // Undo/Redo (Ctrl/Cmd+Z, Ctrl/Cmd+Shift+Z)
    Shortcut {
        sequence: StandardKey.Undo
        onActivated: controller.undo()
    }
    
    Shortcut {
        sequence: StandardKey.Redo
        onActivated: controller.redo()
    }
    
    // Zoom (Ctrl/Cmd +/-)
    Shortcut {
        sequences: ["Ctrl+=", "Ctrl++", "Meta+=", "Meta++"]
        onActivated: canvasView.zoomIn()
    }
    
    Shortcut {
        sequences: ["Ctrl+-", "Meta+-"]
        onActivated: canvasView.zoomOut()
    }
    
    // Escape - сбросить инструменты и выделение
    Shortcut {
        sequence: "Escape"
        onActivated: {
            canvasView.exitResizeMode()
            canvasView.exitCropMode()
            controller.clearSelection()
        }
    }
    
    // Resize mode — S
    Shortcut {
        sequence: "S"
        onActivated: canvasView.toggleResizeMode()
    }
    
    // Crop mode — C
    Shortcut {
        sequence: "C"
        onActivated: canvasView.toggleCropMode()
    }
    
    // Select all (Ctrl/Cmd+A)
    Shortcut {
        sequence: StandardKey.SelectAll
        onActivated: controller.selectAll()
    }
    
    // Label — L
    Shortcut {
        sequence: "L"
        onActivated: {
            if (controller.hasSelection) {
                labelDialog.openDialog()
            }
        }
    }
    
    // Arrange — A
    Shortcut {
        sequence: "A"
        onActivated: controller.arrangeAll()
    }
    
    // Диалог ввода подписи
    Dialog {
        id: labelDialog
        title: ""
        width: 380
        height: 170
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.NoButton
        
        function openDialog() {
            labelInput.text = ""
            labelDialog.open()
            labelInput.forceActiveFocus()
        }
        
        background: Rectangle {
            color: Theme.backgroundColor
            border.color: Theme.accentColor
            border.width: 2
            radius: 12
        }
        
        header: Item {
            height: 40
            
            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(Theme.accentColor.r, Theme.accentColor.g, Theme.accentColor.b, 0.15)
                radius: 12
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 12
                    color: parent.color
                }
            }
            
            Text {
                anchors.centerIn: parent
                text: "Подпись"
                font.pixelSize: 14
                font.bold: true
                color: Theme.accentColor
            }
        }
        
        contentItem: Item {
            Column {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 12
                
                TextField {
                    id: labelInput
                    width: parent.width
                    placeholderText: "Введите подпись..."
                    font.pixelSize: 14
                    color: Theme.textColor
                    
                    background: Rectangle {
                        color: Theme.controlBackground
                        border.color: labelInput.activeFocus ? Theme.accentColor : Theme.borderColor
                        border.width: 1
                        radius: 6
                    }
                    
                    Keys.onReturnPressed: {
                        controller.setLabelForSelected(labelInput.text)
                        labelDialog.close()
                    }
                    Keys.onEnterPressed: {
                        controller.setLabelForSelected(labelInput.text)
                        labelDialog.close()
                    }
                }
                
                Row {
                    spacing: 10
                    anchors.right: parent.right
                    
                    AbstractButton {
                        width: 90
                        height: 32
                        
                        background: Rectangle {
                            color: "transparent"
                            border.color: Theme.borderColor
                            border.width: 1
                            radius: 6
                        }
                        
                        contentItem: Text {
                            text: "Очистить"
                            color: Theme.textColor
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            controller.setLabelForSelected("")
                            labelDialog.close()
                        }
                    }
                    
                    AbstractButton {
                        width: 90
                        height: 32
                        
                        background: Rectangle {
                            color: Theme.accentColor
                            radius: 6
                        }
                        
                        contentItem: Text {
                            text: "Применить"
                            color: Theme.backgroundColor
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            controller.setLabelForSelected(labelInput.text)
                            labelDialog.close()
                        }
                    }
                }
            }
        }
    }
}
