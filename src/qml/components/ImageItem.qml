//ImageItem.qml — отображение одного изображения на холсте. Поддерживает выделение, перемещение и изменение размера

import QtQuick
import QtQuick.Controls
import ImagoRef

Item {
    id: root

    //свойства из модели (CanvasView)
    required property string itemId
    required property url source
    
    //роли модели (расширенные)
    required property real modelCropX
    required property real modelCropY
    required property real modelCropWidth
    required property real modelCropHeight
    required property string modelLabel
    required property real modelOpacity
    
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
    property bool isWorkspaceLocked: false

    required property BoardController controller

    width: itemWidth
    height: itemHeight

    //изображение
    Image {
        id: image
        anchors.fill: parent
        source: root.imageSource
        fillMode: Image.Stretch
        smooth: true
        mipmap: true
        opacity: root.modelOpacity
        
        // Custom ImageProviders don't support sourceClipRect, so we pass crop params via URL
        // and the provider crops it. To prevent double-cropping bugs if Qt ever changes behavior,
        // we skip sourceClipRect for our custom provider.
        property bool isCustomProvider: root.imageSource.toString().indexOf("image://imago/") === 0
        
        //неразрушающая обрезка
        sourceClipRect: (!isCustomProvider && root.modelCropWidth > 0 && root.modelCropHeight > 0) ? Qt.rect(root.modelCropX, root.modelCropY, root.modelCropWidth, root.modelCropHeight) : undefined
    }

    //подпись над изображением
    Rectangle {
        id: labelBackground
        visible: root.modelLabel !== ""
        
        //компенсируем поворот — текст всегда горизонтален
        rotation: -root.rotation
        transformOrigin: Item.TopLeft
        
        //вычисляем позицию визуального верхнего левого угла bounding box в локальных координатах
        property real _rad: root.rotation * Math.PI / 180.0
        property real _cos: Math.cos(_rad)
        property real _sin: Math.sin(_rad)
        //4 угла элемента в координатах сцены (относительно центра)
        property real _cx: root.itemWidth / 2
        property real _cy: root.itemHeight / 2
        //смещения углов от центра, повернутые
        property var _corners: [
            { sx: -_cx * _cos + _cy * _sin, sy: -_cx * _sin - _cy * _cos }, // top-left
            { sx:  _cx * _cos + _cy * _sin, sy:  _cx * _sin - _cy * _cos }, // top-right
            { sx:  _cx * _cos - _cy * _sin, sy:  _cx * _sin + _cy * _cos }, // bottom-right
            { sx: -_cx * _cos - _cy * _sin, sy: -_cx * _sin + _cy * _cos }  // bottom-left
        ]
        //bounding box min в coordinatах сцены (от центра)
        property real _bbMinX: Math.min(_corners[0].sx, _corners[1].sx, _corners[2].sx, _corners[3].sx)
        property real _bbMinY: Math.min(_corners[0].sy, _corners[1].sy, _corners[2].sy, _corners[3].sy)
        //позиция в сцене (top-left bounding box) — пересчитываем обратно в локальные координаты
        //localPos = rotate(-θ, sceneOffset) + center
        property real _labelSceneX: _cx + _bbMinX
        property real _labelSceneY: _cy + _bbMinY - height - 4 / root.zoomLevel
        //обратный поворот из сцены в локальные координаты
        property real _offX: _labelSceneX - _cx
        property real _offY: _labelSceneY - _cy
        property real _localX: _offX * _cos + _offY * _sin + _cx
        property real _localY: -_offX * _sin + _offY * _cos + _cy
        
        x: _localX
        y: _localY
        width: labelText.implicitWidth + 14 / root.zoomLevel
        height: labelText.implicitHeight + 8 / root.zoomLevel
        color: Qt.rgba(0, 0, 0, 0.65)
        radius: 4 / root.zoomLevel

        Text {
            id: labelText
            anchors.centerIn: parent
            text: root.modelLabel
            font.pixelSize: SettingsManager.labelFontSize / root.zoomLevel
            color: "white"
        }
    }

    //рамка при наведении (полупрозрачная белая)
    Rectangle {
        id: hoverBorder
        anchors.fill: parent
        color: "transparent"
        border.color: Qt.rgba(1, 1, 1, 0.5)
        border.width: (!root.selected && hoverArea.containsMouse && !root.isWorkspaceLocked) ? Math.max(2 / zoomLevel, 1) : 0
        visible: !root.selected && hoverArea.containsMouse && !root.isWorkspaceLocked
    }

    //рамка выделения (яркий акцентный цвет)
    Rectangle {
        id: selectionBorder
        anchors.fill: parent
        anchors.margins: -2 / zoomLevel
        color: "transparent"
        border.color: ThemeManager.colors.accentColor
        border.width: (root.selected && !root.isWorkspaceLocked) ? Math.max(3 / zoomLevel, 2) : 0
        visible: root.selected && !root.isWorkspaceLocked
        radius: 2 / zoomLevel
    }

    //внутренняя белая рамка для контраста
    Rectangle {
        id: selectionBorderInner
        anchors.fill: parent
        color: "transparent"
        border.color: "white"
        border.width: (root.selected && !root.isWorkspaceLocked) ? Math.max(1 / zoomLevel, 1) : 0
        visible: root.selected && !root.isWorkspaceLocked
    }

    //оверлей изменения размера
    ResizeOverlay {
        id: resizeHandles
        visible: root.selected && root.resizeMode
        target: root
        zoomLevel: root.zoomLevel
        controller: root.controller
        itemIndex: root.itemIndex
    }

    //оверлей обрезки
    CropOverlay {
        id: cropOverlay
        visible: root.cropMode && root.selected
        target: root
        zoomLevel: root.zoomLevel
        controller: root.controller
        itemIndex: root.itemIndex

        onCropApplied: function(cropX, cropY, cropWidth, cropHeight) {
            controller.toolController.cropImage(itemIndex, cropX, cropY, cropWidth, cropHeight)
            root.exitCropMode()
        }

        onCropCancelled: {
            root.exitCropMode()
        }
    }

    //hover-зона — только для отслеживания наведения (без перехвата событий)
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
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
