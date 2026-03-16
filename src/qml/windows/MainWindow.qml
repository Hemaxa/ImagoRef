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
    
    // Свойство для режима закрепления
    property bool isPinned: controller.toolController.isPinned
    property bool isPinnedAndInactive: isPinned && !Qt.application.active
    
    // Блокировка рабочей области, когда панель скрыта
    readonly property bool isWorkspaceLocked: toolbar.state === "hidden"
    
    // Режим регулирования прозрачности
    property bool opacityModeActive: false
    
    //создает экземпляр холста
    CanvasView {
        id: canvasView
        anchors.fill: parent
        controller: root.controller //передаем контроллер дальше в холст
        isWorkspaceLocked: root.isWorkspaceLocked
        z: 0
    }
    
    //создает экземпляр панели инструментов
    FloatingToolbar {
        id: toolbar
        controller: root.controller
        visible: !root.isPinnedAndInactive
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
        onOpacityModeClicked: root.opacityModeActive = !root.opacityModeActive
        onPasteClicked: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.clipboardController.pasteFromClipboard(scenePos.x, scenePos.y)
        }
        onArrangeClicked: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.toolController.arrangeAll(scenePos.x, scenePos.y)
        }

        resizeModeActive: canvasView.resizeMode
        cropModeActive: canvasView.cropMode
        opacityModeActive: root.opacityModeActive
        
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
        onActivated: {
            toolbar.state = (toolbar.state === "visible") ? "hidden" : "visible"
            if (toolbar.state === "hidden") {
                controller.selectionController.clearSelection()
            }
        }
    }
    
    //удалить выделенные
    Shortcut {
        sequences: ["Delete", "Backspace"]
        enabled: !root.isWorkspaceLocked
        onActivated: controller.toolController.deleteSelected()
    }
    
    //вставить из буфера
    Shortcut {
        sequences: [StandardKey.Paste]
        enabled: !root.isWorkspaceLocked
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
        enabled: !root.isWorkspaceLocked
        onActivated: controller.toolController.snapToGrid()
    }
    
    //поворот по часовой стрелке
    Shortcut {
        sequence: "R"
        enabled: !root.isWorkspaceLocked
        onActivated: controller.toolController.rotateSelected(90)
    }
    
    //поворот против часовой стрелки
    Shortcut {
        sequence: "Shift+R"
        enabled: !root.isWorkspaceLocked
        onActivated: controller.toolController.rotateSelected(-90)
    }
    
    //действие отмены
    Shortcut {
        sequences: [StandardKey.Undo]
        enabled: !root.isWorkspaceLocked
        onActivated: controller.undo()
    }
    
    //действия возврата
    Shortcut {
        sequences: [StandardKey.Redo]
        enabled: !root.isWorkspaceLocked
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
        enabled: !root.isWorkspaceLocked
        onActivated: canvasView.toggleResizeMode()
    }
    
    //режим обрезки
    Shortcut {
        sequence: "C"
        enabled: !root.isWorkspaceLocked
        onActivated: canvasView.toggleCropMode()
    }
    
    //режим непрозрачности
    Shortcut {
        sequence: "O"
        enabled: !root.isWorkspaceLocked && controller.selectionController.hasSelection
        onActivated: root.opacityModeActive = !root.opacityModeActive
    }
    
    //выбрать все
    Shortcut {
        sequences: [StandardKey.SelectAll]
        enabled: !root.isWorkspaceLocked
        onActivated: controller.selectionController.selectAll()
    }
    
    //надпись
    Shortcut {
        sequence: "L"
        enabled: !root.isWorkspaceLocked
        onActivated: {
            if (controller.selectionController.hasSelection) {
                labelDialog.openDialog()
            }
        }
    }
    
    //расположить
    Shortcut {
        sequence: "A"
        enabled: !root.isWorkspaceLocked
        onActivated: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.toolController.arrangeAll(scenePos.x, scenePos.y)
        }
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
            color: ThemeManager.colors.backgroundColor
            border.color: ThemeManager.colors.accentColor
            border.width: 2
            radius: 12
        }
        
        header: Item {
            height: 40
            
            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(ThemeManager.colors.accentColor.r, ThemeManager.colors.accentColor.g, ThemeManager.colors.accentColor.b, 0.15)
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
                color: ThemeManager.colors.accentColor
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
                    color: ThemeManager.colors.textColor
                    
                    background: Rectangle {
                        color: ThemeManager.colors.controlBackground
                        border.color: labelInput.activeFocus ? ThemeManager.colors.accentColor : ThemeManager.colors.borderColor
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
                            border.color: ThemeManager.colors.borderColor
                            border.width: 1
                            radius: 6
                        }
                        
                        contentItem: Text {
                            text: "Очистить"
                            color: ThemeManager.colors.textColor
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
                            color: ThemeManager.colors.accentColor
                            radius: 6
                        }
                        
                        contentItem: Text {
                            text: "Применить"
                            color: ThemeManager.colors.backgroundColor
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
    
    // Слайдер непрозрачности
    Rectangle {
        id: opacitySliderContainer
        visible: root.opacityModeActive && controller.selectionController.hasSelection && !root.isWorkspaceLocked
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        width: 180
        height: 36
        color: Qt.rgba(ThemeManager.colors.panelColor.r, ThemeManager.colors.panelColor.g, ThemeManager.colors.panelColor.b, 0.95)
        border.color: ThemeManager.colors.borderColor
        border.width: 1
        radius: 8
        z: 100

        Slider {
            id: opacitySlider
            anchors.centerIn: parent
            width: parent.width - 20
            from: 0.1
            to: 1.0
            value: 1.0

            onMoved: {
                controller.toolController.setOpacityForSelected(value)
            }
        }

        Connections {
            target: controller.selectionController
            function onSelectionChanged() {
                if (controller.selectionController.hasSelection) {
                    var indices = controller.model.getSelectedIndices()
                    if (indices.length > 0) {
                        var op = controller.selectionController.getItemOpacity(indices[0])
                        opacitySlider.value = op
                    }
                } else {
                    root.opacityModeActive = false
                }
            }
        }
    }
}
