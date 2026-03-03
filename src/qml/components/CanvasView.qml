import QtQuick
import QtQuick.Controls
import ImagoRef

/**
 * CanvasView.qml — основной холст для отображения изображений.
 * Поддерживает масштабирование, панорамирование, drag-drop,
 * выделение кликом и рамкой (marquee selection).
 */
Item {
    id: root

    required property BoardController controller

    // Уровень зума
    property real zoomLevel: 0.5
    property real minZoom: 0.1
    property real maxZoom: 5.0

    // Режимы инструментов
    property bool resizeMode: false
    property bool cropMode: false

    // Паттерн сетки ("dots", "cross", "none")
    property string canvasPattern: Settings.canvasPattern

    // Размер сцены
    readonly property real sceneSize: 20000

    // Публичные методы зума
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
        // Обновляем contentWidth/Height после зума
        flickable.contentWidth = sceneSize * zoomLevel
        flickable.contentHeight = sceneSize * zoomLevel
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
        if (resizeMode) {
            cropMode = false
            limitSelectionToOne()
        }
    }

    function exitResizeMode() {
        resizeMode = false
    }

    function toggleCropMode() {
        cropMode = !cropMode
        if (cropMode) {
            resizeMode = false
            limitSelectionToOne()
        }
    }

    function exitCropMode() {
        cropMode = false
    }

    // Ограничивает выделение одним элементом (для инструментов resize/crop)
    function limitSelectionToOne() {
        var found = false
        for (var i = 0; i < controller.model.count; i++) {
            if (controller.isItemSelected(i)) {
                if (found) {
                    controller.deselectItem(i)
                } else {
                    found = true
                }
            }
        }
    }

    // Фон рабочей области
    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.backgroundColor
    }

    // Прокручиваемая область
    Flickable {
        id: flickable
        anchors.fill: parent

        contentWidth: sceneSize * zoomLevel
        contentHeight: sceneSize * zoomLevel
        clip: true

        Component.onCompleted: {
            contentX = sceneSize * zoomLevel / 2 - width / 2
            contentY = sceneSize * zoomLevel / 2 - height / 2
        }

        interactive: false

        // Контейнер сцены с масштабированием
        Item {
            id: sceneContainer
            width: sceneSize
            height: sceneSize
            scale: zoomLevel
            transformOrigin: Item.TopLeft

            // Паттерн сетки — тайловый Image
            // Сдвигаем на половину клетки, чтобы точки (которые в центре тайла) попадали на (0,0)
            Image {
                id: gridPattern
                anchors.fill: parent
                anchors.margins: root.canvasPattern === "dots" || root.canvasPattern === "cross" ? -patternCanvas.gSize / 2 : 0
                z: 0
                fillMode: Image.Tile
                visible: root.canvasPattern !== "none"
                source: patternCanvas.ready ? patternCanvas.toDataURL() : ""
                smooth: false
            }

            // Генератор тайла паттерна
            Canvas {
                id: patternCanvas
                visible: false
                width: Math.max(Settings.gridSize, 20)
                height: Math.max(Settings.gridSize, 20)

                property bool ready: false
                property color dotColor: Theme.gridColor
                property string pattern: root.canvasPattern
                property int gSize: width

                onPaint: {
                    var ctx = getContext("2d")
                    var s = width
                    ctx.clearRect(0, 0, s, s)
                    ctx.fillStyle = dotColor
                    ctx.strokeStyle = dotColor

                    if (pattern === "dots") {
                        // Точка в ЦЕНТРЕ тайла (меньше артефактов)
                        ctx.beginPath()
                        ctx.arc(s / 2, s / 2, 1.5, 0, 2 * Math.PI)
                        ctx.fill()
                    } else if (pattern === "cross") {
                        // Крестик в ЦЕНТРЕ тайла
                        var cx = s / 2
                        var cy = s / 2
                        var arm = 3
                        ctx.lineWidth = 1
                        ctx.beginPath()
                        ctx.moveTo(cx - arm, cy)
                        ctx.lineTo(cx + arm, cy)
                        ctx.moveTo(cx, cy - arm)
                        ctx.lineTo(cx, cy + arm)
                        ctx.stroke()
                    }
                    ready = true
                }

                // Перерисовка тайла при изменении параметров
                onDotColorChanged: { ready = false; requestPaint() }
                onPatternChanged: { ready = false; requestPaint() }
                onGSizeChanged: { width = Math.max(gSize, 20); height = Math.max(gSize, 20); ready = false; requestPaint() }
                Component.onCompleted: requestPaint()
            }

            // Визуальная граница рабочей области
            Rectangle {
                id: sceneBorder
                anchors.fill: parent
                color: "transparent"
                border.color: Qt.rgba(Theme.accentColor.r, Theme.accentColor.g, Theme.accentColor.b, 0.4)
                border.width: 3 / zoomLevel
                z: 2
            }

            // Изображения (z=10 и выше)
            Repeater {
                id: imagesRepeater
                model: controller.model

                delegate: ImageItem {
                    id: imgDelegate
                    z: 10 + index

                    // Роли модели (переименовали selected -> modelSelected во избежание конфликтов)
                    // Note: 'index' is not a role, so we use context property 'index'
                    // Properties 'itemId', 'source', 'modelCrop*' are defined in ImageItem.qml
                    // we DO NOT need to redeclare them here, Repeater will set them on the base component.
                    
                    required property real modelX
                    required property real modelY
                    required property real modelWidth
                    required property real modelHeight
                    required property real modelRotation
                    required property bool modelSelected // Роль из модели (теперь modelSelected)

                    // Применяем свойства
                    x: imgDelegate.modelX
                    y: imgDelegate.modelY
                    itemWidth: imgDelegate.modelWidth
                    itemHeight: imgDelegate.modelHeight
                    rotation: imgDelegate.modelRotation
                    
                    itemIndex: index // Bind context property 'index' to itemIndex
                    
                    // Привязываем свойство компонента ImageItem.selected к роли модели
                    selected: imgDelegate.modelSelected
                    imageSource: imgDelegate.source
                    resizeMode: root.resizeMode && imgDelegate.modelSelected
                    cropMode: root.cropMode && imgDelegate.modelSelected
                    controller: root.controller
                    zoomLevel: root.zoomLevel

                    // Перемещение только через drag, отключаем в режимах инструментов
                    enableDrag: !root.resizeMode && !root.cropMode

                    onExitCropMode: root.cropMode = false
                }
            }
        }
    }

    // Рамка выделения (marquee selection) — поверх Flickable
    Rectangle {
        id: selectionRect
        visible: false
        color: Qt.rgba(
            Theme.accentColor.r, Theme.accentColor.g, Theme.accentColor.b, 0.15
        )
        border.color: Theme.accentColor
        border.width: 1
        z: 1000
    }



    // Единая MouseArea для выделения и панорамирования
    // z: -1 чтобы элементы управления (Resize/Crop) могли перехватывать события
    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        hoverEnabled: true
        z: -1

        property point pressPos        // Позиция нажатия в координатах экрана
        property point lastPos         // Последняя позиция (для панорамирования)
        property bool isPanning: false
        property bool isSelecting: false
        property bool isDragging: false     // Перетаскивание выделенного изображения
        property int dragIndex: -1         // Индекс перетаскиваемого элемента
        property point dragStartScene      // Начальная позиция мыши в сцене при начале drag
        property real dragItemStartX: 0    // Начальная X позиция элемента при начале drag
        property real dragItemStartY: 0    // Начальная Y позиция элемента при начале drag

        onPressed: function(mouse) {
            if (mouse.button === Qt.MiddleButton) {
                // Панорамирование средней кнопкой
                isPanning = true
                lastPos = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
                return
            }

            // ЛКМ — определяем, что под курсором
            pressPos = Qt.point(mouse.x, mouse.y)
            var scenePos = mapToScene(pressPos)
            var hitIdx = controller.hitTest(scenePos.x, scenePos.y)

            if (hitIdx >= 0) {
                // Клик по изображению
                if (resizeMode || cropMode) {
                    // В режиме инструмента — всегда переключаем выделение на кликнутый элемент
                    controller.selectItem(hitIdx, false)
                } else if (mouse.modifiers & Qt.ControlModifier) {
                    // Ctrl — убрать из выделения
                    controller.deselectItem(hitIdx)
                } else if (mouse.modifiers & Qt.ShiftModifier) {
                    // Shift — добавить к выделению
                    controller.selectItem(hitIdx, true)
                } else {
                    // Обычный клик — единственное выделение (если не уже выделен)
                    if (!controller.isItemSelected(hitIdx)) {
                        controller.selectItem(hitIdx, false)
                    }
                }

                // Drag только если инструменты не активны
                if (!resizeMode && !cropMode) {
                    isDragging = true
                    dragIndex = hitIdx
                    dragStartScene = scenePos
                    dragItemStartX = controller.getItemX(hitIdx)
                    dragItemStartY = controller.getItemY(hitIdx)
                    controller.beginMove(hitIdx)
                    cursorShape = Qt.ClosedHandCursor
                }
            } else {
                // Клик по пустому месту — начинаем рамочное выделение
                // Деактивируем режимы инструментов
                resizeMode = false
                cropMode = false

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
                // Панорамирование
                var dx = mouse.x - lastPos.x
                var dy = mouse.y - lastPos.y
                flickable.contentX -= dx
                flickable.contentY -= dy
                lastPos = Qt.point(mouse.x, mouse.y)
            } else if (isDragging && dragIndex >= 0) {
                // Перемещение элемента — дельта от начальной точки мыши
                var currentScene = mapToScene(Qt.point(mouse.x, mouse.y))
                var deltaX = currentScene.x - dragStartScene.x
                var deltaY = currentScene.y - dragStartScene.y
                var newX = dragItemStartX + deltaX
                var newY = dragItemStartY + deltaY

                // Clamping к границам рабочей области
                var itemW = controller.getItemWidth(dragIndex)
                var itemH = controller.getItemHeight(dragIndex)
                if (itemW <= 0) itemW = 100
                if (itemH <= 0) itemH = 100
                newX = Math.max(0, Math.min(sceneSize - itemW, newX))
                newY = Math.max(0, Math.min(sceneSize - itemH, newY))

                controller.model.updatePosition(dragIndex, newX, newY)
                cursorShape = Qt.ClosedHandCursor
            } else if (isSelecting) {
                // Обновляем рамку выделения
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
            } else if (isDragging) {
                isDragging = false
                cursorShape = Qt.ArrowCursor

                // Создаём undo-команду если позиция изменилась
                if (dragIndex >= 0) {
                    var finalX = controller.getItemX(dragIndex)
                    var finalY = controller.getItemY(dragIndex)
                    controller.endMove(dragIndex, finalX, finalY)
                }
                dragIndex = -1
            } else if (isSelecting) {
                isSelecting = false
                selectionRect.visible = false

                // Если рамка достаточно большая, выполняем выделение
                if (selectionRect.width > 5 && selectionRect.height > 5) {
                    var topLeft = mapToScene(Qt.point(selectionRect.x, selectionRect.y))
                    var bottomRight = mapToScene(Qt.point(
                        selectionRect.x + selectionRect.width,
                        selectionRect.y + selectionRect.height
                    ))

                    var rectX = Math.min(topLeft.x, bottomRight.x)
                    var rectY = Math.min(topLeft.y, bottomRight.y)
                    var rectW = Math.abs(bottomRight.x - topLeft.x)
                    var rectH = Math.abs(bottomRight.y - topLeft.y)

                    controller.selectInRect(rectX, rectY, rectW, rectH,
                                            mouse.modifiers & Qt.ShiftModifier)
                } else if (selectionRect.width < 3 && selectionRect.height < 3) {
                    // Мини-клик по пустому месту — очищаем выделение
                    if (!(mouse.modifiers & Qt.ShiftModifier)) {
                        controller.clearSelection()
                    }
                }
            }
        }
    }

    // Обработка колёсика мыши для зума и прокрутки
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

    // Drag & Drop файлов
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
