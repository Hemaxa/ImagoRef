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
                    lastPos = mapToItem(target.parent, mouse.x, mouse.y)
                    root.startPos = Qt.point(controller.getItemX(itemIndex), controller.getItemY(itemIndex))
                    root.startSize = Qt.size(controller.getItemWidth(itemIndex), controller.getItemHeight(itemIndex))
                    controller.beginResize(itemIndex)
                }
                
                onPositionChanged: function(mouse) {
                    if (!pressed) return
                    
                    var pos = mapToItem(target.parent, mouse.x, mouse.y)
                    var sceneDx = pos.x - lastPos.x
                    var sceneDy = pos.y - lastPos.y
                    lastPos = pos
                    
                    // Поворот: переводим дельту из сцены в локальные координаты
                    var rad = target.rotation * Math.PI / 180.0
                    var cosR = Math.cos(rad)
                    var sinR = Math.sin(rad)
                    var localDx =  sceneDx * cosR + sceneDy * sinR
                    var localDy = -sceneDx * sinR + sceneDy * cosR
                    
                    var x = controller.getItemX(itemIndex)
                    var y = controller.getItemY(itemIndex)
                    var w = controller.getItemWidth(itemIndex)
                    var h = controller.getItemHeight(itemIndex)
                    var cx = x + w / 2
                    var cy = y + h / 2
                    
                    // Якорная точка (противоположный угол/сторона) в локальных координатах от центра
                    var anchorOffX = 0, anchorOffY = 0
                    switch (handleType) {
                        case 0: anchorOffX =  w/2; anchorOffY =  h/2; break // TopLeft → BottomRight
                        case 1: anchorOffX =  0;   anchorOffY =  h/2; break // Top → Bottom center
                        case 2: anchorOffX = -w/2; anchorOffY =  h/2; break // TopRight → BottomLeft
                        case 3: anchorOffX = -w/2; anchorOffY =  0;   break // Right → Left center
                        case 4: anchorOffX = -w/2; anchorOffY = -h/2; break // BottomRight → TopLeft
                        case 5: anchorOffX =  0;   anchorOffY = -h/2; break // Bottom → Top center
                        case 6: anchorOffX =  w/2; anchorOffY = -h/2; break // BottomLeft → TopRight
                        case 7: anchorOffX =  w/2; anchorOffY =  0;   break // Left → Right center
                    }
                    
                    // Позиция якоря в координатах сцены
                    var anchorSceneX = cx + anchorOffX * cosR - anchorOffY * sinR
                    var anchorSceneY = cy + anchorOffX * sinR + anchorOffY * cosR
                    
                    // Изменение размера в локальных координатах
                    var dw = 0, dh = 0
                    switch (handleType) {
                        case 0: dw = -localDx; dh = -localDy; break
                        case 1: dh = -localDy; break
                        case 2: dw =  localDx; dh = -localDy; break
                        case 3: dw =  localDx; break
                        case 4: dw =  localDx; dh =  localDy; break
                        case 5: dh =  localDy; break
                        case 6: dw = -localDx; dh =  localDy; break
                        case 7: dw = -localDx; break
                    }
                    
                    var newW = w + dw
                    var newH = h + dh
                    
                    // Пропорциональность (Shift)
                    var keepAspect = mouse.modifiers & Qt.ShiftModifier
                    var aspectRatio = startSize.width / startSize.height
                    if (keepAspect) {
                        if (handleType === 1 || handleType === 5) {
                            newW = newH * aspectRatio
                        } else if (handleType === 3 || handleType === 7) {
                            newH = newW / aspectRatio
                        } else {
                            if (Math.abs(localDx) > Math.abs(localDy)) {
                                newH = newW / aspectRatio
                            } else {
                                newW = newH * aspectRatio
                            }
                        }
                    }
                    
                    if (newW < 20 || newH < 20) return
                    
                    // Новое смещение якоря от нового центра
                    var newAnchorOffX = 0, newAnchorOffY = 0
                    switch (handleType) {
                        case 0: newAnchorOffX =  newW/2; newAnchorOffY =  newH/2; break
                        case 1: newAnchorOffX =  0;      newAnchorOffY =  newH/2; break
                        case 2: newAnchorOffX = -newW/2; newAnchorOffY =  newH/2; break
                        case 3: newAnchorOffX = -newW/2; newAnchorOffY =  0;      break
                        case 4: newAnchorOffX = -newW/2; newAnchorOffY = -newH/2; break
                        case 5: newAnchorOffX =  0;      newAnchorOffY = -newH/2; break
                        case 6: newAnchorOffX =  newW/2; newAnchorOffY = -newH/2; break
                        case 7: newAnchorOffX =  newW/2; newAnchorOffY =  0;      break
                    }
                    
                    // Новый центр: якорь должен остаться на месте
                    var newCx = anchorSceneX - newAnchorOffX * cosR + newAnchorOffY * sinR
                    var newCy = anchorSceneY - newAnchorOffX * sinR - newAnchorOffY * cosR
                    var newX = newCx - newW / 2
                    var newY = newCy - newH / 2
                    
                    controller.model.updatePosition(itemIndex, newX, newY)
                    controller.model.updateSize(itemIndex, newW, newH)
                }
                
                onReleased: {
                    // Читаем финальные значения из модели
                    var finalX = controller.getItemX(itemIndex)
                    var finalY = controller.getItemY(itemIndex)
                    var finalW = controller.getItemWidth(itemIndex)
                    var finalH = controller.getItemHeight(itemIndex)
                    
                    if (finalX !== startPos.x || finalY !== startPos.y ||
                        finalW !== startSize.width || finalH !== startSize.height) {
                        controller.endResize(itemIndex, finalX, finalY, finalW, finalH)
                    }
                }
            }
        }
    }
}

