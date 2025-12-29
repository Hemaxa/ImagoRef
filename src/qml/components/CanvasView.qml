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
    }
    
    function exitResizeMode() {
        resizeMode = false
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
            
            // Сетка точек - оптимизированная версия с debounce
            Item {
                id: gridLayer
                anchors.fill: parent
                
                property int gridSize: Math.max(controller.gridSize, 20)
                
                // Таймер для debounce перерисовки сетки
                Timer {
                    id: gridUpdateTimer
                    interval: 50
                    onTriggered: gridRepeater.updateGrid()
                }
                
                Repeater {
                    id: gridRepeater
                    
                    property real viewX: 0
                    property real viewY: 0
                    property real viewWidth: 0
                    property real viewHeight: 0
                    property int gSize: gridLayer.gridSize
                    
                    function updateGrid() {
                        viewX = flickable.contentX / zoomLevel
                        viewY = flickable.contentY / zoomLevel
                        viewWidth = flickable.width / zoomLevel
                        viewHeight = flickable.height / zoomLevel
                    }
                    
                    property int startCol: Math.floor(viewX / gSize)
                    property int startRow: Math.floor(viewY / gSize)
                    property int cols: Math.ceil(viewWidth / gSize) + 2
                    property int rows: Math.ceil(viewHeight / gSize) + 2
                    property int totalDots: Math.min(cols * rows, 2500)
                    
                    model: totalDots
                    
                    delegate: Rectangle {
                        required property int index
                        
                        property int col: gridRepeater.startCol + (index % gridRepeater.cols)
                        property int row: gridRepeater.startRow + Math.floor(index / gridRepeater.cols)
                        
                        x: col * gridRepeater.gSize
                        y: row * gridRepeater.gSize
                        width: 3
                        height: 3
                        radius: 1.5
                        color: Theme.gridColor
                    }
                }
                
                Connections {
                    target: flickable
                    function onContentXChanged() { gridUpdateTimer.restart() }
                    function onContentYChanged() { gridUpdateTimer.restart() }
                }
                
                Component.onCompleted: gridRepeater.updateGrid()
            }
            
            // Изображения
            Repeater {
                id: imagesRepeater
                model: controller.model
                
                delegate: ImageItem {
                    id: imgDelegate
                    
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
                    controller: root.controller
                    zoomLevel: root.zoomLevel
                }
            }
        }
    }
    
    // Обработка колесика мыши (зум)
    WheelHandler {
        onWheel: function(event) {
            if (event.modifiers & Qt.ControlModifier) {
                var factor = event.angleDelta.y > 0 ? 1.15 : 1/1.15
                var newZoom = Math.max(minZoom, Math.min(maxZoom, zoomLevel * factor))
                setZoom(newZoom, event.point.position)
                event.accepted = true
            } else {
                event.accepted = false
            }
        }
    }
    
    // Обработка мыши (панорамирование средней кнопкой)
    MouseArea {
        id: panMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.MiddleButton | Qt.LeftButton
        
        property point lastPos
        property bool isPanning: false
        
        onPressed: function(mouse) {
            if (mouse.button === Qt.MiddleButton) {
                isPanning = true
                lastPos = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
            } else if (mouse.button === Qt.LeftButton) {
                controller.clearSelection()
            }
        }
        
        onPositionChanged: function(mouse) {
            if (isPanning) {
                var dx = mouse.x - lastPos.x
                var dy = mouse.y - lastPos.y
                flickable.contentX -= dx
                flickable.contentY -= dy
                lastPos = Qt.point(mouse.x, mouse.y)
            }
        }
        
        onReleased: function(mouse) {
            if (mouse.button === Qt.MiddleButton) {
                isPanning = false
                cursorShape = Qt.ArrowCursor
            }
        }
        
        propagateComposedEvents: true
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
    
    // Обновляем сетку при изменении зума
    onZoomLevelChanged: gridUpdateTimer.restart()
}
