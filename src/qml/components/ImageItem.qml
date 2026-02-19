import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * ImageItem.qml - отображение одного изображения на холсте.
 * Поддерживает выделение, перемещение и изменение размера.
 */
Item {
    id: root
    
    // Свойства из модели
    property int itemIndex: -1
    property url imageSource
    property real itemWidth: 100
    property real itemHeight: 100
    property bool selected: false
    property bool resizeMode: false
    property bool cropMode: false
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
    }
    
    // Рамка при наведении (серая)
    Rectangle {
        id: hoverBorder
        anchors.fill: parent
        color: "transparent"
        border.color: Qt.rgba(1, 1, 1, 0.5)
        border.width: (!root.selected && mouseArea.containsMouse) ? Math.max(2 / zoomLevel, 1) : 0
        visible: !root.selected && mouseArea.containsMouse
    }
    
    // Рамка выделения (яркая синяя)
    Rectangle {
        id: selectionBorder
        anchors.fill: parent
        anchors.margins: -2 / zoomLevel
        color: "transparent"
        border.color: "#3B82F6"
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
    
    // Маркеры изменения размера - показываются когда объект выделен
    ResizeHandles {
        id: resizeHandles
        visible: root.selected
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
        }
        
        onCropCancelled: {
            root.cropMode = false
        }
    }
    
    // Обработка мыши
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        drag.target: (root.resizeMode || root.cropMode) ? null : root
        drag.threshold: 0
        
        property point pressPos
        property point itemStartPos
        
        onPressed: function(mouse) {
            // Ctrl — toggle выделения
            if (mouse.modifiers & Qt.ControlModifier) {
                controller.toggleSelection(itemIndex)
            }
            // Shift — добавить к выделению
            else if (mouse.modifiers & Qt.ShiftModifier) {
                controller.selectItem(itemIndex, true)
            }
            // Обычный клик
            else if (!root.selected) {
                controller.selectItem(itemIndex, false)
            }
            
            // Запоминаем начальную позицию для undo (только если не Ctrl)
            if (!(mouse.modifiers & Qt.ControlModifier)) {
                pressPos = Qt.point(mouse.x, mouse.y)
                itemStartPos = Qt.point(root.x, root.y)
                controller.beginMove(itemIndex)
            }
        }
        
        onReleased: function(mouse) {
            // Создаем undo команду если позиция изменилась
            if (root.x !== itemStartPos.x || root.y !== itemStartPos.y) {
                controller.endMove(itemIndex, root.x, root.y)
                // Обновляем модель
                controller.model.updatePosition(itemIndex, root.x, root.y)
            }
        }
        
        cursorShape: (resizeMode || cropMode) ? Qt.ArrowCursor : 
                     (drag.active ? Qt.ClosedHandCursor : 
                     (containsMouse ? Qt.OpenHandCursor : Qt.ArrowCursor))
    }
}
