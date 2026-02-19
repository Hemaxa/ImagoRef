import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * CanvasView.qml - основной холст для отображения изображений.
 * Поддерживает масштабирование, панорамирование и drag-drop.
 * Оптимизировано для производительности.
 */
Item {
    id: root
    
    required property BoardController controller
    
    // Уровень зума
    property real zoomLevel: 1.0
    property real minZoom: 0.1
    property real maxZoom: 5.0
    
    // Режим ресайза
    property bool resizeMode: false
    
    // Режим обрезки
    property bool cropMode: false
    
    // Публичные методы
    function zoomIn() {
        var newZoom = Math.min(zoomLevel * 1.15, maxZoom)
        setZoom(newZoom, Qt.point(width / 2, height / 2))
    }
    
    function zoomOut() {
        var newZoom = Math.max(zoomLevel / 1.15, minZoom)
        setZoom(newZoom, Qt.point(width / 2, height / 2))
    }
    
    function setZoom(newZoom, center) {
        var scenePos = mapToScene(center)
        zoomLevel = newZoom
        flickable.contentX = scenePos.x * zoomLevel - center.x
        flickable.contentY = scenePos.y * zoomLevel - center.y
    }
    
    function mapToScene(point) {
        return Qt.point(
            (flickable.contentX + point.x) / zoomLevel,
            (flickable.contentY + point.y) / zoomLevel
        )
    }
    
    function toggleResizeMode() {
        resizeMode = !resizeMode
        if (resizeMode) cropMode = false
    }
    
    function exitResizeMode() {
        resizeMode = false
    }
    
    function toggleCropMode() {
        cropMode = !cropMode
        if (cropMode) resizeMode = false
    }
    
    function exitCropMode() {
        cropMode = false
    }
    
    // Фон
    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.backgroundColor
    }
    
    // Прокручиваемая область
    Flickable {
        id: flickable
        anchors.fill: parent
        
        contentWidth: 10000
        contentHeight: 10000
        clip: true
        
        Component.onCompleted: {
            contentX = 5000 - width / 2
            contentY = 5000 - height / 2
        }
        
        interactive: false
        
        // Контейнер сцены с масштабированием
        Item {
            id: sceneContainer
            width: 10000
            height: 10000
            scale: zoomLevel
            transformOrigin: Item.TopLeft
            
            // Область для клика по пустому месту (z=0)
            MouseArea {
                id: backgroundClickArea
                anchors.fill: parent
                z: 0
                acceptedButtons: Qt.LeftButton
                onClicked: controller.clearSelection()
            }
            
            // Сетка точек (z=1) - использует Canvas для производительности
            Canvas {
                id: gridCanvas
                anchors.fill: parent
                z: 1
                
                property int gSize: Math.max(controller.gridSize, 20)
                property color dotColor: Theme.gridColor
                
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    
                    var viewX = flickable.contentX / zoomLevel
                    var viewY = flickable.contentY / zoomLevel
                    var viewW = flickable.width / zoomLevel
                    var viewH = flickable.height / zoomLevel
                    
                    var startCol = Math.floor(viewX / gSize)
                    var startRow = Math.floor(viewY / gSize)
                    var endCol = Math.ceil((viewX + viewW) / gSize) + 1
                    var endRow = Math.ceil((viewY + viewH) / gSize) + 1
                    
                    ctx.fillStyle = dotColor
                    for (var col = startCol; col <= endCol; col++) {
                        for (var row = startRow; row <= endRow; row++) {
                            ctx.beginPath()
                            ctx.arc(col * gSize, row * gSize, 1.5, 0, 2 * Math.PI)
                            ctx.fill()
                        }
                    }
                }
                
                // Перерисовка при изменении параметров
                onGSizeChanged: requestPaint()
                onDotColorChanged: requestPaint()
                
                Connections {
                    target: flickable
                    function onContentXChanged() { gridCanvas.requestPaint() }
                    function onContentYChanged() { gridCanvas.requestPaint() }
                    function onWidthChanged() { gridCanvas.requestPaint() }
                    function onHeightChanged() { gridCanvas.requestPaint() }
                }
                
                Connections {
                    target: root
                    function onZoomLevelChanged() { gridCanvas.requestPaint() }
                }
                
                Component.onCompleted: requestPaint()
            }
            
            // Изображения (z=10 и выше)
            Repeater {
                id: imagesRepeater
                model: controller.model
                
                delegate: ImageItem {
                    id: imgDelegate
                    z: 10 + imgDelegate.index
                    
                    // Required properties для ролей модели
                    required property int index
                    required property string itemId
                    required property url source
                    required property real modelX
                    required property real modelY 
                    required property real modelWidth
                    required property real modelHeight
                    required property real modelRotation
                    required property bool selected
                    
                    // Применяем свойства
                    x: imgDelegate.modelX
                    y: imgDelegate.modelY
                    itemWidth: imgDelegate.modelWidth
                    itemHeight: imgDelegate.modelHeight
                    rotation: imgDelegate.modelRotation
                    itemIndex: imgDelegate.index
                    selected: imgDelegate.selected
                    imageSource: imgDelegate.source
                    resizeMode: root.resizeMode && imgDelegate.selected
                    cropMode: root.cropMode && imgDelegate.selected
                    controller: root.controller
                    zoomLevel: root.zoomLevel
                }
            }
        }
    }
    
    // Рамка выделения (marquee selection)
    Rectangle {
        id: selectionRect
        visible: false
        color: Qt.rgba(0.3, 0.5, 0.9, 0.2)
        border.color: Qt.rgba(0.3, 0.5, 0.9, 0.8)
        border.width: 1
        z: 1000
    }
    
    // Обработка выделения рамкой и панорамирования
    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        hoverEnabled: true
        
        property point pressPos
        property point lastPos
        property bool isPanning: false
        property bool isSelecting: false
        
        onPressed: function(mouse) {
            if (mouse.button === Qt.MiddleButton) {
                // Панорамирование средней кнопкой
                isPanning = true
                lastPos = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            } else if (mouse.button === Qt.LeftButton) {
                // Начало выделения рамкой
                pressPos = Qt.point(mouse.x, mouse.y)
                isSelecting = true
                selectionRect.x = mouse.x
                selectionRect.y = mouse.y
                selectionRect.width = 0
                selectionRect.height = 0
                selectionRect.visible = true
                
                // Очищаем выделение если не зажат Shift
                if (!(mouse.modifiers & Qt.ShiftModifier)) {
                    controller.clearSelection()
                }
            }
        }
        
        onPositionChanged: function(mouse) {
            if (isPanning) {
                var dx = mouse.x - lastPos.x
                var dy = mouse.y - lastPos.y
                flickable.contentX -= dx
                flickable.contentY -= dy
                lastPos = Qt.point(mouse.x, mouse.y)
            } else if (isSelecting) {
                // Обновляем размер рамки выделения
                var minX = Math.min(pressPos.x, mouse.x)
                var minY = Math.min(pressPos.y, mouse.y)
                var maxX = Math.max(pressPos.x, mouse.x)
                var maxY = Math.max(pressPos.y, mouse.y)
                
                selectionRect.x = minX
                selectionRect.y = minY
                selectionRect.width = maxX - minX
                selectionRect.height = maxY - minY
            }
        }
        
        onReleased: function(mouse) {
            if (isPanning) {
                isPanning = false
                cursorShape = Qt.ArrowCursor
            } else if (isSelecting) {
                isSelecting = false
                selectionRect.visible = false
                
                // Если рамка достаточно большая, выполняем выделение
                if (selectionRect.width > 5 && selectionRect.height > 5) {
                    // Преобразуем координаты экрана в координаты сцены
                    var topLeft = mapToScene(Qt.point(selectionRect.x, selectionRect.y))
                    var bottomRight = mapToScene(Qt.point(selectionRect.x + selectionRect.width, 
                                                          selectionRect.y + selectionRect.height))
                    
                    var rectX = Math.min(topLeft.x, bottomRight.x)
                    var rectY = Math.min(topLeft.y, bottomRight.y)
                    var rectW = Math.abs(bottomRight.x - topLeft.x)
                    var rectH = Math.abs(bottomRight.y - topLeft.y)
                    
                    controller.selectInRect(rectX, rectY, rectW, rectH, mouse.modifiers & Qt.ShiftModifier)
                } else if (selectionRect.width < 3 && selectionRect.height < 3) {
                    // Простой клик - очищаем выделение (если не Shift)
                    if (!(mouse.modifiers & Qt.ShiftModifier)) {
                        controller.clearSelection()
                    }
                }
            }
        }
    }
    
    // Обработка колесика мыши для зума
    MouseArea {
        id: wheelArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        
        onWheel: function(wheel) {
            if (wheel.modifiers & Qt.ControlModifier) {
                // Зум с Ctrl
                var factor = wheel.angleDelta.y > 0 ? 1.15 : 1/1.15
                var newZoom = Math.max(minZoom, Math.min(maxZoom, zoomLevel * factor))
                setZoom(newZoom, Qt.point(wheel.x, wheel.y))
            } else {
                // Прокрутка без Ctrl
                flickable.contentX -= wheel.angleDelta.x
                flickable.contentY -= wheel.angleDelta.y
            }
        }
    }
    
    // Drag & Drop
    DropArea {
        anchors.fill: parent
        
        onDropped: function(drop) {
            if (drop.hasUrls) {
                for (var i = 0; i < drop.urls.length; i++) {
                    var scenePos = mapToScene(Qt.point(drop.x, drop.y))
                    controller.addImage(drop.urls[i], scenePos.x, scenePos.y)
                }
            }
        }
    }
}
