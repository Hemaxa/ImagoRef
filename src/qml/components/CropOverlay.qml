import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * CropOverlay.qml - оверлей для обрезки изображения.
 * Показывает затемнённую область вне обрезки и ручки для изменения.
 */
Item {
    id: root
    
    required property Item target
    required property BoardController controller
    property int itemIndex: -1
    property real zoomLevel: 1.0
    
    // Сигналы
    signal cropApplied(real cropX, real cropY, real cropWidth, real cropHeight)
    signal cropCancelled()
    
    // Область обрезки (в координатах изображения)
    property real cropX: target.width * 0.1
    property real cropY: target.height * 0.1
    property real cropW: target.width * 0.8
    property real cropH: target.height * 0.8
    
    property real handleSize: 12 / zoomLevel
    property real minCropSize: 20
    
    anchors.fill: parent
    
    // Затемнение сверху
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: cropY
        color: Qt.rgba(0, 0, 0, 0.6)
    }
    
    // Затемнение снизу
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height - cropY - cropH
        color: Qt.rgba(0, 0, 0, 0.6)
    }
    
    // Затемнение слева
    Rectangle {
        anchors.left: parent.left
        y: cropY
        width: cropX
        height: cropH
        color: Qt.rgba(0, 0, 0, 0.6)
    }
    
    // Затемнение справа
    Rectangle {
        anchors.right: parent.right
        y: cropY
        width: parent.width - cropX - cropW
        height: cropH
        color: Qt.rgba(0, 0, 0, 0.6)
    }
    
    // Рамка области обрезки
    Rectangle {
        id: cropRect
        x: cropX
        y: cropY
        width: cropW
        height: cropH
        color: "transparent"
        border.color: "white"
        border.width: 2 / zoomLevel
        
        // Сетка третей (правило третей)
        Rectangle {
            x: parent.width / 3
            y: 0
            width: 1 / zoomLevel
            height: parent.height
            color: Qt.rgba(1, 1, 1, 0.4)
        }
        Rectangle {
            x: parent.width * 2 / 3
            y: 0
            width: 1 / zoomLevel
            height: parent.height
            color: Qt.rgba(1, 1, 1, 0.4)
        }
        Rectangle {
            x: 0
            y: parent.height / 3
            width: parent.width
            height: 1 / zoomLevel
            color: Qt.rgba(1, 1, 1, 0.4)
        }
        Rectangle {
            x: 0
            y: parent.height * 2 / 3
            width: parent.width
            height: 1 / zoomLevel
            color: Qt.rgba(1, 1, 1, 0.4)
        }
        
        // Перетаскивание всей области обрезки
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor
            
            property point lastPos
            
            onPressed: function(mouse) {
                lastPos = Qt.point(mouse.x, mouse.y)
            }
            
            onPositionChanged: function(mouse) {
                if (!pressed) return
                
                var dx = mouse.x - lastPos.x
                var dy = mouse.y - lastPos.y
                
                var newX = Math.max(0, Math.min(target.width - cropW, cropX + dx))
                var newY = Math.max(0, Math.min(target.height - cropH, cropY + dy))
                
                cropX = newX
                cropY = newY
            }
        }
    }
    
    // Угловые ручки
    Repeater {
        model: 4
        
        Rectangle {
            id: cornerHandle
            property int corner: index  // 0=TL, 1=TR, 2=BR, 3=BL
            
            width: handleSize
            height: handleSize
            color: "white"
            border.color: "#3B82F6"
            border.width: 2 / zoomLevel
            
            x: (corner === 0 || corner === 3) ? cropX - handleSize/2 : cropX + cropW - handleSize/2
            y: (corner === 0 || corner === 1) ? cropY - handleSize/2 : cropY + cropH - handleSize/2
            
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 / zoomLevel
                cursorShape: (corner === 0 || corner === 2) ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor
                
                property point lastPos
                
                onPressed: function(mouse) {
                    lastPos = mapToItem(root, mouse.x, mouse.y)
                }
                
                onPositionChanged: function(mouse) {
                    if (!pressed) return
                    
                    var pos = mapToItem(root, mouse.x, mouse.y)
                    var dx = pos.x - lastPos.x
                    var dy = pos.y - lastPos.y
                    lastPos = pos
                    
                    var newX = cropX
                    var newY = cropY
                    var newW = cropW
                    var newH = cropH
                    
                    switch(corner) {
                        case 0: // TopLeft
                            newX = Math.max(0, Math.min(cropX + cropW - minCropSize, cropX + dx))
                            newY = Math.max(0, Math.min(cropY + cropH - minCropSize, cropY + dy))
                            newW = cropW - (newX - cropX)
                            newH = cropH - (newY - cropY)
                            break
                        case 1: // TopRight
                            newY = Math.max(0, Math.min(cropY + cropH - minCropSize, cropY + dy))
                            newW = Math.max(minCropSize, Math.min(target.width - cropX, cropW + dx))
                            newH = cropH - (newY - cropY)
                            break
                        case 2: // BottomRight
                            newW = Math.max(minCropSize, Math.min(target.width - cropX, cropW + dx))
                            newH = Math.max(minCropSize, Math.min(target.height - cropY, cropH + dy))
                            break
                        case 3: // BottomLeft
                            newX = Math.max(0, Math.min(cropX + cropW - minCropSize, cropX + dx))
                            newW = cropW - (newX - cropX)
                            newH = Math.max(minCropSize, Math.min(target.height - cropY, cropH + dy))
                            break
                    }
                    
                    cropX = newX
                    cropY = newY
                    cropW = newW
                    cropH = newH
                }
            }
        }
    }
    
    // Боковые ручки
    Repeater {
        model: 4
        
        Rectangle {
            id: sideHandle
            property int side: index  // 0=Top, 1=Right, 2=Bottom, 3=Left
            
            width: (side === 0 || side === 2) ? handleSize * 2 : handleSize / 2
            height: (side === 0 || side === 2) ? handleSize / 2 : handleSize * 2
            color: "white"
            border.color: "#3B82F6"
            border.width: 1 / zoomLevel
            
            x: {
                switch(side) {
                    case 0: case 2: return cropX + cropW/2 - width/2
                    case 1: return cropX + cropW - width/2
                    case 3: return cropX - width/2
                }
            }
            y: {
                switch(side) {
                    case 0: return cropY - height/2
                    case 2: return cropY + cropH - height/2
                    case 1: case 3: return cropY + cropH/2 - height/2
                }
            }
            
            MouseArea {
                anchors.fill: parent
                anchors.margins: -5 / zoomLevel
                cursorShape: (side === 0 || side === 2) ? Qt.SizeVerCursor : Qt.SizeHorCursor
                
                property point lastPos
                
                onPressed: function(mouse) {
                    lastPos = mapToItem(root, mouse.x, mouse.y)
                }
                
                onPositionChanged: function(mouse) {
                    if (!pressed) return
                    
                    var pos = mapToItem(root, mouse.x, mouse.y)
                    var dx = pos.x - lastPos.x
                    var dy = pos.y - lastPos.y
                    lastPos = pos
                    
                    switch(side) {
                        case 0: // Top
                            var newY = Math.max(0, Math.min(cropY + cropH - minCropSize, cropY + dy))
                            cropH = cropH - (newY - cropY)
                            cropY = newY
                            break
                        case 1: // Right
                            cropW = Math.max(minCropSize, Math.min(target.width - cropX, cropW + dx))
                            break
                        case 2: // Bottom
                            cropH = Math.max(minCropSize, Math.min(target.height - cropY, cropH + dy))
                            break
                        case 3: // Left
                            var newX = Math.max(0, Math.min(cropX + cropW - minCropSize, cropX + dx))
                            cropW = cropW - (newX - cropX)
                            cropX = newX
                            break
                    }
                }
            }
        }
    }
    
    // Кнопки управления
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -40 / zoomLevel
        spacing: 10 / zoomLevel
        z: 100
        
        Button {
            width: 80 / zoomLevel
            height: 30 / zoomLevel
            text: "✓ Применить"
            font.pixelSize: 12 / zoomLevel
            
            background: Rectangle {
                color: "#22C55E"
                radius: 4 / zoomLevel
            }
            
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.font.pixelSize
            }
            
            onClicked: {
                // Передаём координаты относительно отображаемого размера
                // C++ сам конвертирует в координаты оригинального изображения
                root.cropApplied(cropX, cropY, cropW, cropH)
            }
        }
        
        Button {
            width: 80 / zoomLevel
            height: 30 / zoomLevel
            text: "✕ Отмена"
            font.pixelSize: 12 / zoomLevel
            
            background: Rectangle {
                color: "#EF4444"
                radius: 4 / zoomLevel
            }
            
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: parent.font.pixelSize
            }
            
            onClicked: root.cropCancelled()
        }
    }
}
