import QtQuick
import ImagoRef

/**
 * ResizeHandles.qml - маркеры изменения размера для ImageItem.
 * 8 маркеров по углам и сторонам.
 */
Item {
    id: root
    
    required property Item target
    required property BoardController controller
    property int itemIndex: -1
    property real zoomLevel: 1.0
    
    property real handleSize: 12 / zoomLevel
    
    anchors.fill: parent
    
    // Начальные значения для undo
    property point startPos
    property size startSize
    
    // Enum для типов маркеров
    enum HandleType {
        TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left
    }
    
    // Маркеры
    Repeater {
        model: 8
        
        Rectangle {
            id: handle
            width: handleSize
            height: handleSize
            radius: handleSize / 2
            color: "white"
            border.color: "black"
            border.width: 2 / zoomLevel
            
            property int handleType: index
            
            x: getX(handleType)
            y: getY(handleType)
            
            function getX(type) {
                switch (type) {
                    case 0: case 6: case 7: return -handleSize / 2  // Left
                    case 1: case 5: return target.width / 2 - handleSize / 2  // Center
                    case 2: case 3: case 4: return target.width - handleSize / 2  // Right
                }
                return 0
            }
            
            function getY(type) {
                switch (type) {
                    case 0: case 1: case 2: return -handleSize / 2  // Top
                    case 3: case 7: return target.height / 2 - handleSize / 2  // Center
                    case 4: case 5: case 6: return target.height - handleSize / 2  // Bottom
                }
                return 0
            }
            
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 / zoomLevel  // Увеличенная область захвата
                
                property point lastPos
                
                cursorShape: getCursor(handleType)
                
                function getCursor(type) {
                    switch (type) {
                        case 0: case 4: return Qt.SizeFDiagCursor  // TopLeft, BottomRight
                        case 2: case 6: return Qt.SizeBDiagCursor  // TopRight, BottomLeft
                        case 1: case 5: return Qt.SizeVerCursor   // Top, Bottom
                        case 3: case 7: return Qt.SizeHorCursor   // Right, Left
                    }
                    return Qt.ArrowCursor
                }
                
                onPressed: function(mouse) {
                    lastPos = mapToItem(root.parent, mouse.x, mouse.y)
                    root.startPos = Qt.point(target.x, target.y)
                    root.startSize = Qt.size(target.width, target.height)
                    controller.beginResize(itemIndex)
                }
                
                onPositionChanged: function(mouse) {
                    if (!pressed) return
                    
                    var pos = mapToItem(root.parent, mouse.x, mouse.y)
                    var dx = (pos.x - lastPos.x) / zoomLevel
                    var dy = (pos.y - lastPos.y) / zoomLevel
                    lastPos = pos
                    
                    var newX = target.x
                    var newY = target.y
                    var newW = target.itemWidth
                    var newH = target.itemHeight
                    
                    // Пропорциональное изменение при Shift
                    var keepAspect = mouse.modifiers & Qt.ShiftModifier
                    var aspectRatio = startSize.width / startSize.height
                    
                    switch (handleType) {
                        case 0: // TopLeft
                            newX += dx; newY += dy
                            newW -= dx; newH -= dy
                            break
                        case 1: // Top
                            newY += dy; newH -= dy
                            break
                        case 2: // TopRight
                            newY += dy
                            newW += dx; newH -= dy
                            break
                        case 3: // Right
                            newW += dx
                            break
                        case 4: // BottomRight
                            newW += dx; newH += dy
                            break
                        case 5: // Bottom
                            newH += dy
                            break
                        case 6: // BottomLeft
                            newX += dx
                            newW -= dx; newH += dy
                            break
                        case 7: // Left
                            newX += dx; newW -= dx
                            break
                    }
                    
                    // Пропорциональность
                    if (keepAspect) {
                        if (handleType === 1 || handleType === 5) {
                            newW = newH * aspectRatio
                        } else if (handleType === 3 || handleType === 7) {
                            newH = newW / aspectRatio
                        } else {
                            if (Math.abs(dx) > Math.abs(dy)) {
                                newH = newW / aspectRatio
                            } else {
                                newW = newH * aspectRatio
                            }
                        }
                    }
                    
                    // Минимальный размер
                    if (newW >= 20 && newH >= 20) {
                        target.x = newX
                        target.y = newY
                        target.itemWidth = newW
                        target.itemHeight = newH
                    }
                }
                
                onReleased: {
                    if (target.x !== startPos.x || target.y !== startPos.y ||
                        target.itemWidth !== startSize.width || target.itemHeight !== startSize.height) {
                        controller.endResize(itemIndex, target.x, target.y, target.itemWidth, target.itemHeight)
                        controller.model.updatePosition(itemIndex, target.x, target.y)
                        controller.model.updateSize(itemIndex, target.itemWidth, target.itemHeight)
                    }
                }
            }
        }
    }
}
