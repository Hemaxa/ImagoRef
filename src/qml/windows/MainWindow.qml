//MainWindow.qml - главное окно приложения с холстом и панелью инструментов

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef

import "../components"

Item {
    id: root
    
    //получаем контроллер доски из Main.qml
    required property BoardController controller
    signal settingsRequested()
    
    //создает экземпляр холста
    CanvasView {
        id: canvasView
        anchors.fill: parent
        controller: root.controller //передаем контроллер дальше в холст
        z: 0
    }
    
    //создает экземпляр панели инструментов
    FloatingToolbar {
        id: toolbar
        controller: root.controller
        z: 100
        x: 15
        y: 15
        
        //подключаем сигналы нажатия кнопок на панели и активации режимов редактирования
        onSettingsClicked: root.settingsRequested()
        onZoomInClicked: canvasView.zoomIn()
        onZoomOutClicked: canvasView.zoomOut()
        onResizeModeClicked: canvasView.toggleResizeMode()
        onCropModeClicked: canvasView.toggleCropMode()
        onLabelClicked: labelDialog.openDialog()
        onPasteClicked: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.clipboardController.pasteFromClipboard(scenePos.x, scenePos.y)
        }

        resizeModeActive: canvasView.resizeMode
        cropModeActive: canvasView.cropMode
        
        //состояния видимости панели для анимации
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
    
    //горячие клавиши
    //показать или скрыть панель
    Shortcut {
        sequence: "Tab"
        onActivated: toolbar.state = (toolbar.state === "visible") ? "hidden" : "visible"
    }
    
    //удалить выделенные
    Shortcut {
        sequences: ["Delete", "Backspace"]
        onActivated: controller.toolController.deleteSelected()
    }
    
    //вставить из буфера
    Shortcut {
        sequence: StandardKey.Paste
        onActivated: {
            //вычисляется центр экрана для вставки ровно по центру
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.clipboardController.pasteFromClipboard(scenePos.x, scenePos.y)
        }
    }
    
    //привязка к сетке
    Shortcut {
        sequence: "G"
        onActivated: controller.toolController.snapToGrid()
    }
    
    //поворот по часовой стрелке
    Shortcut {
        sequence: "R"
        onActivated: controller.toolController.rotateSelected(90)
    }
    
    //поворот против часовой стрелки
    Shortcut {
        sequence: "Shift+R"
        onActivated: controller.toolController.rotateSelected(-90)
    }
    
    //действие отмены
    Shortcut {
        sequence: StandardKey.Undo
        onActivated: controller.undo()
    }
    
    //действия возврата
    Shortcut {
        sequence: StandardKey.Redo
        onActivated: controller.redo()
    }
    
    //приблизить
    Shortcut {
        sequences: ["Ctrl+=", "Ctrl++", "Meta+=", "Meta++"]
        onActivated: canvasView.zoomIn()
    }
    
    //отдалить
    Shortcut {
        sequences: ["Ctrl+-", "Meta+-"]
        onActivated: canvasView.zoomOut()
    }
    
    //сбросить инструменты и выделение
    Shortcut {
        sequence: "Escape"
        onActivated: {
            canvasView.exitResizeMode()
            canvasView.exitCropMode()
            controller.selectionController.clearSelection()
        }
    }
    
    //режим изменения размера
    Shortcut {
        sequence: "S"
        onActivated: canvasView.toggleResizeMode()
    }
    
    //режим обрезки
    Shortcut {
        sequence: "C"
        onActivated: canvasView.toggleCropMode()
    }
    
    //выбрать все
    Shortcut {
        sequence: StandardKey.SelectAll
        onActivated: controller.selectionController.selectAll()
    }
    
    //надпись
    Shortcut {
        sequence: "L"
        onActivated: {
            if (controller.selectionController.hasSelection) {
                labelDialog.openDialog()
            }
        }
    }
    
    //расположить
    Shortcut {
        sequence: "A"
        onActivated: controller.toolController.arrangeAll()
    }
    
    //диалог ввода подписи
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
            color: ThemeManager.backgroundColor
            border.color: ThemeManager.accentColor
            border.width: 2
            radius: 12
        }
        
        header: Item {
            height: 40
            
            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(ThemeManager.accentColor.r, ThemeManager.accentColor.g, ThemeManager.accentColor.b, 0.15)
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
                color: ThemeManager.accentColor
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
                    color: ThemeManager.textColor
                    
                    background: Rectangle {
                        color: ThemeManager.controlBackground
                        border.color: labelInput.activeFocus ? ThemeManager.accentColor : ThemeManager.borderColor
                        border.width: 1
                        radius: 6
                    }
                    
                    Keys.onReturnPressed: {
                        controller.toolController.setLabelForSelected(labelInput.text)
                        labelDialog.close()
                    }
                    Keys.onEnterPressed: {
                        controller.toolController.setLabelForSelected(labelInput.text)
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
                            border.color: ThemeManager.borderColor
                            border.width: 1
                            radius: 6
                        }
                        
                        contentItem: Text {
                            text: "Очистить"
                            color: ThemeManager.textColor
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            controller.toolController.setLabelForSelected("")
                            labelDialog.close()
                        }
                    }
                    
                    AbstractButton {
                        width: 90
                        height: 32
                        
                        background: Rectangle {
                            color: ThemeManager.accentColor
                            radius: 6
                        }
                        
                        contentItem: Text {
                            text: "Применить"
                            color: ThemeManager.backgroundColor
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            controller.toolController.setLabelForSelected(labelInput.text)
                            labelDialog.close()
                        }
                    }
                }
            }
        }
    }
}
