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
    signal arrangeClicked()
    signal opacityModeClicked()
    
    //состояние синхронизации
    property bool isCloudSyncing: false
    property string syncProgressText: ""

    Connections {
        target: controller.cloudController
        function onCloudSyncStarted() {
            root.isCloudSyncing = true
            root.syncProgressText = "Синхронизация..."
        }
        function onCloudSyncProgress(current, total) {
            root.syncProgressText = current + " / " + total
        }
        function onCloudSyncFinished() {
            root.isCloudSyncing = false
        }
    }
    
    //cостояние активных инструментов
    property bool resizeModeActive: false
    property bool cropModeActive: false
    property bool opacityModeActive: false
    property bool labelModeActive: false
    
    // Cвойства для отслеживания состояния кнопок
    property bool btnZoomInVisible: true
    property bool btnZoomOutVisible: true
    property bool btnResizeVisible: true
    property bool btnCropVisible: true
    property bool btnLabelVisible: true
    property bool btnOpacityVisible: true
    property bool btnArrangeVisible: true
    property bool btnEyedropperVisible: true
    property bool btnPasteVisible: true
    property bool btnDeleteVisible: true
    property bool btnGridSnapVisible: true
    property bool btnUpscaleVisible: true
    property bool btnRotateVisible: true
    property bool btnUndoRedoVisible: true
    property bool btnPinVisible: true
    property bool btnSettingsBtnVisible: true
    
    // Оставляем только нужные свойства
    property int columns: SettingsManager.toolbarColumns
    property int itemSpacing: 3
    property int paddingVal: 5

    // БИНГО! Привязываем размер фона к реальному размеру сетки (implicitWidth/Height)
    width: toolbarLayout.implicitWidth + (paddingVal * 2)
    height: toolbarLayout.implicitHeight + (paddingVal * 2)

    color: Qt.rgba(ThemeManager.colors.panelColor.r, ThemeManager.colors.panelColor.g, ThemeManager.colors.panelColor.b, 1.0)
    border.color: ThemeManager.colors.borderColor
    border.width: 1
    radius: 3

    // Drag handle for resizing horizontally
    MouseArea {
        id: rightResizeHandle
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 15
        cursorShape: Qt.SizeHorCursor
        
        property int startX: 0
        property int startColumns: 1
        
        onPressed: (mouse) => {
            startX = mouse.x
            startColumns = SettingsManager.toolbarColumns
        }
        
        onPositionChanged: (mouse) => {
            let dx = mouse.x - startX
            
            // To prevent flickering (switching back and forth over a single pixel boundary):
            // We use a significant threshold (e.g. half a button width = 25px)
            if (startColumns === 1 && dx > 25) { 
                SettingsManager.toolbarColumns = 2
                // Do not update startX or startColumns, so they have to drag ALL the way back
                // past the original point + threshold to flip it back.
            } else if (startColumns === 2 && dx < -25) { 
                SettingsManager.toolbarColumns = 1
            }
        }
    }
    
    GridLayout {
        id: toolbarLayout
        columns: root.columns
        anchors.centerIn: parent
        rowSpacing: root.itemSpacing
        columnSpacing: root.itemSpacing
        
        // Вставить
        ToolbarButton {
            iconSource: ThemeManager.icons.pasteIcon
            tooltip: "Вставить"
            shortcutText: "Ctrl+V"
            visible: root.btnPasteVisible
            onClicked: root.pasteClicked()
        }
        
        // Удалить
        ToolbarButton {
            iconSource: ThemeManager.icons.deleteIcon
            tooltip: "Удалить"
            shortcutText: "Delete"
            enabled: controller.selectionController.hasSelection
            visible: root.btnDeleteVisible
            onClicked: controller.toolController.deleteSelected()
        }
        
        // Привязать к сетке
        ToolbarButton {
            iconSource: ThemeManager.icons.gridSnapIcon
            tooltip: "Привязать к сетке"
            shortcutText: "G"
            enabled: controller.selectionController.hasSelection
            visible: root.btnGridSnapVisible
            onClicked: controller.toolController.snapToGrid()
        }
        
        // Увеличить разрешение
        ToolbarButton {
            iconSource: ThemeManager.icons.upscaleIcon
            tooltip: "Увеличить разрешение"
            shortcutText: "U"
            visible: ModelsManager.isModelDownloaded && root.btnUpscaleVisible
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
            iconSource: ThemeManager.icons.scaleIcon
            tooltip: "Изменить размер"
            shortcutText: "S"
            active: root.resizeModeActive
            visible: root.btnResizeVisible
            enabled: controller.selectionController.hasSelection
            onClicked: root.resizeModeClicked()
        }
        
        // Обрезать
        ToolbarButton {
            iconSource: ThemeManager.icons.cropIcon
            tooltip: "Обрезать"
            shortcutText: "C"
            active: root.cropModeActive
            visible: root.btnCropVisible
            enabled: controller.selectionController.hasSelection
            onClicked: root.cropModeClicked()
        }

        // Пипетка
        ToolbarButton {
            iconSource: ThemeManager.icons.eyedropperIcon
            tooltip: "Пипетка"
            shortcutText: "I"
            active: controller.toolController.isEyedropperActive
            visible: root.btnEyedropperVisible
            onClicked: controller.toolController.toggleEyedropper()
        }
        
        // Непрозрачность
        ToolbarButton {
            iconSource: ThemeManager.icons.opacityIcon
            tooltip: "Непрозрачность"
            shortcutText: "O"
            active: root.opacityModeActive
            visible: root.btnOpacityVisible
            enabled: controller.selectionController.hasSelection
            onClicked: root.opacityModeClicked()
        }
        
        // Подписать
        ToolbarButton {
            iconSource: ThemeManager.icons.labelIcon
            tooltip: "Подписать"
            shortcutText: "L"
            enabled: controller.selectionController.hasSelection
            active: root.labelModeActive
            visible: root.btnLabelVisible
            onClicked: root.labelClicked()
        }
        
        // Расположить
        ToolbarButton {
            iconSource: ThemeManager.icons.arrangeIcon
            tooltip: "Расположить"
            shortcutText: "A"
            visible: root.btnArrangeVisible
            enabled: controller.model.count > 0
            onClicked: root.arrangeClicked()
        }
        
        // Вращать против часовой
        ToolbarButton {
            iconSource: ThemeManager.icons.rotateLeftIcon
            tooltip: "Вращать против часовой"
            shortcutText: "Shift+R"
            enabled: controller.selectionController.hasSelection
            visible: root.btnRotateVisible
            onClicked: controller.toolController.rotateSelected(-90)
        }
        
        // Вращать по часовой
        ToolbarButton {
            iconSource: ThemeManager.icons.rotateRightIcon
            tooltip: "Вращать по часовой"
            shortcutText: "R"
            enabled: controller.selectionController.hasSelection
            visible: root.btnRotateVisible
            onClicked: controller.toolController.rotateSelected(90)
        }
        
        // Приблизить
        ToolbarButton {
            iconSource: ThemeManager.icons.zoomInIcon
            tooltip: "Приблизить"
            shortcutText: "Ctrl++"
            visible: root.btnZoomInVisible
            onClicked: root.zoomInClicked()
        }
        
        // Отдалить
        ToolbarButton {
            iconSource: ThemeManager.icons.zoomOutIcon
            tooltip: "Отдалить"
            shortcutText: "Ctrl+-"
            visible: root.btnZoomOutVisible
            onClicked: root.zoomOutClicked()
        }
        
        // Отменить
        ToolbarButton {
            iconSource: ThemeManager.icons.undoIcon
            tooltip: "Отменить"
            shortcutText: "Ctrl+Z"
            enabled: controller.canUndo
            visible: root.btnUndoRedoVisible
            onClicked: controller.undo()
        }
        
        // Повторить
        ToolbarButton {
            iconSource: ThemeManager.icons.redoIcon
            tooltip: "Повторить"
            shortcutText: "Ctrl+Shift+Z"
            enabled: controller.canRedo
            visible: root.btnUndoRedoVisible
            onClicked: controller.redo()
        }

        // Закрепить поверх окон
        ToolbarButton {
            iconSource: ThemeManager.icons.pinIcon
            tooltip: "Закрепить поверх всех окон"
            active: controller.toolController.isPinned
            visible: root.btnPinVisible
            onClicked: controller.toolController.togglePin()
        }
        
        // Настройки
        ToolbarButton {
            iconSource: ThemeManager.icons.settingsIcon
            tooltip: "Настройки"
            shortcutText: "Ctrl+,"
            visible: root.btnSettingsBtnVisible
            onClicked: root.settingsClicked()
        }
    }

    // Индикатор облачной загрузки поверх панели (только когда идет синхронизация)
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(ThemeManager.colors.panelColor.r, ThemeManager.colors.panelColor.g, ThemeManager.colors.panelColor.b, 0.8)
        radius: root.radius
        visible: root.isCloudSyncing

        ColumnLayout {
            anchors.centerIn: parent
            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                running: root.isCloudSyncing
            }
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: root.syncProgressText
                color: ThemeManager.colors.textColor
                font.pixelSize: 12
            }
        }
        
        // Перехватываем клики, чтобы пользователь не мог нажимать кнопки
        MouseArea {
            anchors.fill: parent
        }
    }
}
