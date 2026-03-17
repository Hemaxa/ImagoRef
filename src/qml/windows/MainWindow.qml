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

    // Панель истории цветов
    property bool colorPaletteActive: false
    
    // Состояния включения инструментов (так как SettingsManager.isToolEnabled не имеет NOTIFY сигнала)
    property bool toolZoomInEnabled: SettingsManager.isToolEnabled("ZoomIn")
    property bool toolZoomOutEnabled: SettingsManager.isToolEnabled("ZoomOut")
    property bool toolResizeEnabled: SettingsManager.isToolEnabled("Resize")
    property bool toolCropEnabled: SettingsManager.isToolEnabled("Crop")
    property bool toolLabelEnabled: SettingsManager.isToolEnabled("Label")
    property bool toolOpacityEnabled: SettingsManager.isToolEnabled("Opacity")
    property bool toolArrangeEnabled: SettingsManager.isToolEnabled("Arrange")
    property bool toolEyedropperEnabled: SettingsManager.isToolEnabled("Eyedropper")
    property bool toolPasteEnabled: SettingsManager.isToolEnabled("Paste")
    property bool toolDeleteEnabled: SettingsManager.isToolEnabled("Delete")
    property bool toolGridSnapEnabled: SettingsManager.isToolEnabled("GridSnap")
    property bool toolUpscaleEnabled: SettingsManager.isToolEnabled("Upscale")
    property bool toolRotateEnabled: SettingsManager.isToolEnabled("Rotate")
    property bool toolUndoRedoEnabled: SettingsManager.isToolEnabled("UndoRedo")
    property bool toolPinEnabled: SettingsManager.isToolEnabled("Pin")
    property bool toolSettingsBtnEnabled: SettingsManager.isToolEnabled("SettingsBtn")
    
    Connections {
        target: SettingsManager
        function onToolEnablementChanged(toolName, enabled) {
            if (toolName === "ZoomIn") root.toolZoomInEnabled = enabled
            else if (toolName === "ZoomOut") root.toolZoomOutEnabled = enabled
            else if (toolName === "Resize") root.toolResizeEnabled = enabled
            else if (toolName === "Crop") root.toolCropEnabled = enabled
            else if (toolName === "Label") root.toolLabelEnabled = enabled
            else if (toolName === "Opacity") root.toolOpacityEnabled = enabled
            else if (toolName === "Arrange") root.toolArrangeEnabled = enabled
            else if (toolName === "Eyedropper") root.toolEyedropperEnabled = enabled
            else if (toolName === "Paste") root.toolPasteEnabled = enabled
            else if (toolName === "Delete") root.toolDeleteEnabled = enabled
            else if (toolName === "GridSnap") root.toolGridSnapEnabled = enabled
            else if (toolName === "Upscale") root.toolUpscaleEnabled = enabled
            else if (toolName === "Rotate") root.toolRotateEnabled = enabled
            else if (toolName === "UndoRedo") root.toolUndoRedoEnabled = enabled
            else if (toolName === "Pin") root.toolPinEnabled = enabled
            else if (toolName === "SettingsBtn") root.toolSettingsBtnEnabled = enabled
        }
    }
    
    // Взаимоисключение для нижних выезжающих панелей
    onColorPaletteActiveChanged: {
        if (root.colorPaletteActive) {
            root.opacityModeActive = false
            root.labelDialogVisilble = false
        }
    }
    
    onOpacityModeActiveChanged: {
        if (root.opacityModeActive) {
            root.colorPaletteActive = false
            root.labelDialogVisilble = false
        }
    }
    
    onLabelDialogVisilbleChanged: {
        if (root.labelDialogVisilble) {
            root.colorPaletteActive = false
            root.opacityModeActive = false
        }
    }
    
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
        
        btnZoomInVisible: root.toolZoomInEnabled
        btnZoomOutVisible: root.toolZoomOutEnabled
        btnResizeVisible: root.toolResizeEnabled
        btnCropVisible: root.toolCropEnabled
        btnLabelVisible: root.toolLabelEnabled
        btnOpacityVisible: root.toolOpacityEnabled
        btnArrangeVisible: root.toolArrangeEnabled
        btnEyedropperVisible: root.toolEyedropperEnabled
        btnPasteVisible: root.toolPasteEnabled
        btnDeleteVisible: root.toolDeleteEnabled
        btnGridSnapVisible: root.toolGridSnapEnabled
        btnUpscaleVisible: root.toolUpscaleEnabled
        btnRotateVisible: root.toolRotateEnabled
        btnUndoRedoVisible: root.toolUndoRedoEnabled
        btnPinVisible: root.toolPinEnabled
        btnSettingsBtnVisible: root.toolSettingsBtnEnabled
        
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
        labelModeActive: root.labelDialogVisilble
        
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
    
    // Панель истории цветов
    ColorPalettePanel {
        id: colorPalette
        controller: root.controller
        isPanelVisible: root.colorPaletteActive && !root.isPinnedAndInactive
        z: 100
    }
    
    // Оверлей пипетки
    EyedropperOverlay {
        id: eyedropperOverlay
        controller: root.controller
        z: 9999
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
        enabled: !root.isWorkspaceLocked && root.toolPasteEnabled
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
        enabled: root.toolZoomInEnabled
        onActivated: canvasView.zoomIn()
    }
    
    //отдалить
    Shortcut {
        sequences: ["Ctrl+-", "Meta+-"]
        enabled: root.toolZoomOutEnabled
        onActivated: canvasView.zoomOut()
    }
    
    //сбросить инструменты и выделение
    Shortcut {
        sequence: "Escape"
        onActivated: {
            canvasView.exitResizeMode()
            canvasView.exitCropMode()
            controller.selectionController.clearSelection()
            
            if (controller.toolController.isEyedropperActive) {
                controller.toolController.toggleEyedropper()
            }
            if (root.labelDialogVisilble) {
                labelDialog.closeDialog()
            }
            if (root.opacityModeActive) {
                root.opacityModeActive = false
            }
        }
    }
    
    //режим изменения размера
    Shortcut {
        sequence: "S"
        enabled: !root.isWorkspaceLocked && root.toolResizeEnabled
        onActivated: canvasView.toggleResizeMode()
    }
    
    //режим обрезки
    Shortcut {
        sequence: "C"
        enabled: !root.isWorkspaceLocked && root.toolCropEnabled
        onActivated: canvasView.toggleCropMode()
    }
    
    //режим непрозрачности
    Shortcut {
        sequence: "O"
        enabled: !root.isWorkspaceLocked && controller.selectionController.hasSelection && root.toolOpacityEnabled
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
        enabled: !root.isWorkspaceLocked && root.toolLabelEnabled
        onActivated: {
            if (controller.selectionController.hasSelection) {
                labelDialog.openDialog()
            }
        }
    }
    
    //расположить
    Shortcut {
        sequence: "A"
        enabled: !root.isWorkspaceLocked && root.toolArrangeEnabled
        onActivated: {
            var center = Qt.point(canvasView.width / 2, canvasView.height / 2)
            var scenePos = canvasView.mapToScene(center)
            controller.toolController.arrangeAll(scenePos.x, scenePos.y)
        }
    }
    
    // Пипетка
    Shortcut {
        sequence: "I"
        enabled: !root.isWorkspaceLocked && root.toolEyedropperEnabled
        onActivated: controller.toolController.toggleEyedropper()
    }
    
    // Панель истории цветов
    Shortcut {
        sequence: "P"
        enabled: !root.isPinnedAndInactive
        onActivated: root.colorPaletteActive = !root.colorPaletteActive
    }
    
    // Диалог ввода подписи
    property bool labelDialogVisilble: false
    Rectangle {
        id: labelDialog
        width: 380
        height: 140
        
        // Позиционируем снизу по центру
        x: (parent.width - width) / 2
        y: root.labelDialogVisilble ? parent.height - height - 20 : parent.height + 20
        
        // Скрываем если панель за границами
        visible: y < parent.height
        
        color: ThemeManager.colors.backgroundColor
        border.color: ThemeManager.colors.accentColor
        border.width: 2
        radius: 12
        z: 200
        
        Behavior on y {
            NumberAnimation {
                duration: 300
                easing.type: Easing.InOutCubic
            }
        }
        
        function openDialog() {
            labelInput.text = ""
            root.labelDialogVisilble = true
            labelInput.forceActiveFocus()
        }
        
        function closeDialog() {
            root.labelDialogVisilble = false
            canvasView.forceActiveFocus()
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12
            
            TextField {
                id: labelInput
                Layout.fillWidth: true
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
                    labelDialog.closeDialog()
                }
                Keys.onEnterPressed: {
                    controller.toolController.setLabelForSelected(labelInput.text)
                    labelDialog.closeDialog()
                }
                Keys.onEscapePressed: {
                    labelDialog.closeDialog()
                }
            }
            
            RowLayout {
                spacing: 10
                Layout.alignment: Qt.AlignRight
                
                AbstractButton {
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 32
                    
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
                        labelDialog.closeDialog()
                    }
                }
                
                AbstractButton {
                    Layout.preferredWidth: 90
                    Layout.preferredHeight: 32
                    
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
                        labelDialog.closeDialog()
                    }
                }
                
                AbstractButton {
                    Layout.preferredWidth: 80
                    Layout.preferredHeight: 32
                    
                    background: Rectangle {
                        color: "transparent"
                        border.color: ThemeManager.colors.borderColor
                        border.width: 1
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: "Отмена"
                        color: ThemeManager.colors.textColor
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        labelDialog.closeDialog()
                    }
                }
            }
        }
    }
    
    // Слайдер непрозрачности
    Rectangle {
        id: opacitySliderContainer
        
        property bool isOpacityVisible: root.opacityModeActive && controller.selectionController.hasSelection && !root.isWorkspaceLocked
        
        width: 180
        height: 36
        
        // Позиционируем снизу по центру
        x: (parent.width - width) / 2
        y: isOpacityVisible ? parent.height - height - 20 : parent.height + 20
        
        // Скрываем если панель за границами
        visible: y < parent.height
        
        color: Qt.rgba(ThemeManager.colors.panelColor.r, ThemeManager.colors.panelColor.g, ThemeManager.colors.panelColor.b, 0.95)
        border.color: ThemeManager.colors.borderColor
        border.width: 1
        radius: 18 // Округляем до состояния пилюли
        z: 100
        
        Behavior on y {
            NumberAnimation {
                duration: 300
                easing.type: Easing.InOutCubic
            }
        }

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
