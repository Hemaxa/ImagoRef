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
        
        // Если source пустой, используем pixmap из модели
        Component.onCompleted: {
            if (source === "") {
                // Fallback - получить данные из контроллера
            }
        }
    }
    
    // Рамка выделения
    Rectangle {
        id: selectionBorder
        anchors.fill: parent
        color: "transparent"
        border.color: "white"
        border.width: (root.selected || mouseArea.containsMouse) ? 2 / zoomLevel : 0
        visible: root.selected || mouseArea.containsMouse
    }
    
    // Маркеры изменения размера
    ResizeHandles {
        id: resizeHandles
        visible: root.resizeMode
        target: root
        zoomLevel: root.zoomLevel
        controller: root.controller
        itemIndex: root.itemIndex
    }
    
    // Обработка мыши
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        drag.target: root.resizeMode ? null : root
        drag.threshold: 0
        
        property point pressPos
        property point itemStartPos
        
        onPressed: function(mouse) {
            // Выделение
            if (mouse.modifiers & Qt.ControlModifier) {
                controller.selectItem(itemIndex, true)
            } else if (!root.selected) {
                controller.selectItem(itemIndex, false)
            }
            
            // Запоминаем начальную позицию для undo
            pressPos = Qt.point(mouse.x, mouse.y)
            itemStartPos = Qt.point(root.x, root.y)
            controller.beginMove(itemIndex)
        }
        
        onReleased: function(mouse) {
            // Создаем undo команду если позиция изменилась
            if (root.x !== itemStartPos.x || root.y !== itemStartPos.y) {
                controller.endMove(itemIndex, root.x, root.y)
                // Обновляем модель
                controller.model.updatePosition(itemIndex, root.x, root.y)
            }
        }
        
        cursorShape: resizeMode ? Qt.ArrowCursor : 
                     (drag.active ? Qt.ClosedHandCursor : 
                     (containsMouse ? Qt.OpenHandCursor : Qt.ArrowCursor))
    }
}
