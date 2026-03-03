import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * ImageItem.qml — отображение одного изображения на холсте.
 * Поддерживает выделение (визуальное), перемещение (drag) и ресайз.
 * Клик-выделение обрабатывается из CanvasView через hitTest.
 */
Item {
    id: root

    // Свойства из модели
    required property string itemId // если есть
    required property url source
    
    // Новые роли обрезки (используем названия, заданные в roleNames)
    required property real modelCropX
    required property real modelCropY
    required property real modelCropWidth
    required property real modelCropHeight
    
    property int itemIndex: index
    property url imageSource: source
    property real itemWidth: 100
    property real itemHeight: 100
    property bool selected: false
    property bool resizeMode: false
    property bool cropMode: false
    signal exitCropMode()
    property bool enableDrag: true
    property real zoomLevel: 1.0

    required property BoardController controller

    width: itemWidth
    height: itemHeight

    // Изображение
    Image {
        id: image
        anchors.fill: parent
        source: root.imageSource
        fillMode: Image.Stretch
        smooth: true
        mipmap: true
        
        // Non-destructive crop
        sourceClipRect: (root.modelCropWidth > 0 && root.modelCropHeight > 0) 
                        ? Qt.rect(root.modelCropX, root.modelCropY, root.modelCropWidth, root.modelCropHeight) 
                        : undefined
    }

    // Рамка при наведении (полупрозрачная белая)
    Rectangle {
        id: hoverBorder
        anchors.fill: parent
        color: "transparent"
        border.color: Qt.rgba(1, 1, 1, 0.5)
        border.width: (!root.selected && hoverArea.containsMouse) ? Math.max(2 / zoomLevel, 1) : 0
        visible: !root.selected && hoverArea.containsMouse
    }

    // Рамка выделения (яркий акцентный цвет)
    Rectangle {
        id: selectionBorder
        anchors.fill: parent
        anchors.margins: -2 / zoomLevel
        color: "transparent"
        border.color: Theme.accentColor
        border.width: root.selected ? Math.max(3 / zoomLevel, 2) : 0
        visible: root.selected
        radius: 2 / zoomLevel
    }

    // Внутренняя белая рамка для контраста
    Rectangle {
        id: selectionBorderInner
        anchors.fill: parent
        color: "transparent"
        border.color: "white"
        border.width: root.selected ? Math.max(1 / zoomLevel, 1) : 0
        visible: root.selected
    }

    // Маркеры изменения размера
    ResizeHandles {
        id: resizeHandles
        visible: root.selected && root.resizeMode
        target: root
        zoomLevel: root.zoomLevel
        controller: root.controller
        itemIndex: root.itemIndex
    }

    // Оверлей обрезки
    CropOverlay {
        id: cropOverlay
        visible: root.cropMode && root.selected
        target: root
        zoomLevel: root.zoomLevel
        controller: root.controller
        itemIndex: root.itemIndex

        onCropApplied: function(cropX, cropY, cropWidth, cropHeight) {
            controller.cropImage(itemIndex, cropX, cropY, cropWidth, cropHeight)
            root.exitCropMode()
        }

        onCropCancelled: {
            root.exitCropMode()
        }
    }

    // Hover-зона — только для отслеживания наведения (без перехвата событий)
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton  // Не перехватываем клики
        propagateComposedEvents: true

        cursorShape: {
            if (root.resizeMode || root.cropMode)
                return Qt.ArrowCursor
            if (root.selected)
                return Qt.OpenHandCursor
            return Qt.ArrowCursor
        }
    }
}
