//CanvasView.qml — основной холст для отображения изображений. Поддерживает масштабирование, панорамирование, drag-drop, выделение кликом и рамкой

import QtQuick
import QtQuick.Controls
import ImagoRef

Item {
    id: root

    //получаем контроллер доски из MainWindow.qml
    required property BoardController controller

    //уровень зума
    property real zoomLevel: 0.3
    property real minZoom: 0.1
    property real maxZoom: 5.0

    //режимы инструментов
    property bool resizeMode: false
    property bool cropMode: false

    //паттерн сетки ("dots", "cross", "none")
    property string canvasPattern: SettingsManager.canvasPattern

    // Свойство для режима закрепления
    property bool isPinned: controller.toolController.isPinned
    property bool isPinnedAndInactive: isPinned && !Qt.application.active

    //размер сцены
    readonly property real sceneSize: 30000 //30000x30000

    //публичные методы зума
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

    //функция перевода экранных координат в координаты рабочей области
    function mapToScene(point) {
        return Qt.point(
            (flickable.contentX + point.x) / zoomLevel,
            (flickable.contentY + point.y) / zoomLevel
        )
    }

    //функции переключения режимов
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

    //ограничивает выделение одним элементом
    function limitSelectionToOne() {
        var found = false
        for (var i = 0; i < controller.model.count; i++) {
            if (controller.selectionController.getIsItemSelected(i)) {
                if (found) {
                    controller.selectionController.deselectItem(i)
                }
                else {
                    found = true
                }
            }
        }
    }

    //фон рабочей области
    Rectangle {
        id: background
        anchors.fill: parent
        color: root.isPinnedAndInactive ? "transparent" : ThemeManager.backgroundColor
    }

    //прокручиваемая область
    //Flickable это элемент QML, который имеет полосы прокрутки, через которое можно смотреть на большой контент
    Flickable {
        id: flickable
        anchors.fill: parent

        //размер содержимого внутри Flickable динамически меняется при зуме
        contentWidth: sceneSize * zoomLevel
        contentHeight: sceneSize * zoomLevel
        clip: true

        //при запуске камеры центрируется ровно посередине сцены
        Component.onCompleted: {
            contentX = sceneSize * zoomLevel / 2 - width / 2
            contentY = sceneSize * zoomLevel / 2 - height / 2
            
            // Записываем начальное положение
            controller.cameraZoom = zoomLevel
            controller.cameraX = contentX
            controller.cameraY = contentY
        }
        
        // Передаем координаты в C++ при их изменении
        onContentXChanged: controller.cameraX = contentX
        onContentYChanged: controller.cameraY = contentY

        interactive: false
        
    }
        
    // Синхронизация масштаба
    onZoomLevelChanged: controller.cameraZoom = zoomLevel

    // Восстановление позиции камеры при открытии файла
    Connections {
        target: controller.fileController
        function onBoardLoaded() {
            if (controller.cameraX >= 0 && controller.cameraY >= 0) {
                root.zoomLevel = controller.cameraZoom
                flickable.contentX = controller.cameraX
                flickable.contentY = controller.cameraY
            } else {
                root.zoomLevel = 0.3
                flickable.contentX = root.sceneSize * root.zoomLevel / 2 - root.width / 2
                flickable.contentY = root.sceneSize * root.zoomLevel / 2 - root.height / 2
            }
        }
    }

    //контейнер сцены с масштабированием
    Item {
        id: sceneContainer
        parent: flickable.contentItem

            width: sceneSize
            height: sceneSize
            scale: zoomLevel //устанавливаем коэффициент зума для всего содержимого рабочей области
            transformOrigin: Item.TopLeft //точка масштабирования

            //паттерн сетки — тайловый Image
            //сдвигаем на половину клетки, чтобы точки (которые в центре тайла) попадали на (0,0)
            Image {
                id: gridPattern
                anchors.fill: parent
                anchors.margins: root.canvasPattern === "dots" || root.canvasPattern === "cross" ? -patternCanvas.gSize / 2 : 0
                z: 0
                fillMode: Image.Tile
                visible: root.canvasPattern !== "none" && !root.isPinnedAndInactive
                source: patternCanvas.ready ? patternCanvas.toDataURL() : ""
                smooth: false
            }

            //генератор тайла паттерна
            Canvas {
                id: patternCanvas
                visible: false
                width: gSize
                height: gSize

                property bool ready: false
                property color dotColor: ThemeManager.gridColor
                property string pattern: root.canvasPattern
                property int gSize: Math.max(SettingsManager.gridSize, 5)

                onPaint: {
                    var ctx = getContext("2d")
                    var s = width
                    ctx.clearRect(0, 0, s, s)
                    ctx.fillStyle = dotColor
                    ctx.strokeStyle = dotColor

                    //отрисовка паттерна точка
                    if (pattern === "dots") {
                        // Точка в ЦЕНТРЕ тайла (меньше артефактов)
                        ctx.beginPath()
                        ctx.arc(s / 2, s / 2, 2, 0, 2 * Math.PI)
                        ctx.fill()
                    }
                    //отрисовка паттерна крестик
                    else if (pattern === "cross") {
                        var cx = s / 2
                        var cy = s / 2
                        var arm = 3
                        ctx.lineWidth = 1.5
                        ctx.beginPath()
                        ctx.moveTo(cx - arm, cy)
                        ctx.lineTo(cx + arm, cy)
                        ctx.moveTo(cx, cy - arm)
                        ctx.lineTo(cx, cy + arm)
                        ctx.stroke()
                    }
                    ready = true
                }

                //перерисовка тайла при изменении параметров
                onDotColorChanged: { ready = false; requestPaint() }
                onPatternChanged: { ready = false; requestPaint() }
                onGSizeChanged: { ready = false; requestPaint() }
                Component.onCompleted: requestPaint()
            }

            //визуальная граница рабочей области
            Rectangle {
                id: sceneBorder
                anchors.fill: parent
                color: "transparent"
                border.color: Qt.rgba(ThemeManager.accentColor.r, ThemeManager.accentColor.g, ThemeManager.accentColor.b, 0.4)
                border.width: 3 / zoomLevel
                z: 2
                visible: !root.isPinnedAndInactive
            }

            //изображения (z=10 и выше)
            Repeater {
                id: imagesRepeater
                model: controller.model

                delegate: ImageItem {
                    id: imgDelegate
                    
                    required property int index
                    z: 10 + imgDelegate.index

                    //роли модели
                    //required требует обязательного получения этих данных из C++
                    required property real modelX
                    required property real modelY
                    required property real modelWidth
                    required property real modelHeight
                    required property real modelRotation
                    required property bool modelSelected

                    //применяем свойства
                    x: imgDelegate.modelX
                    y: imgDelegate.modelY
                    itemWidth: imgDelegate.modelWidth
                    itemHeight: imgDelegate.modelHeight
                    rotation: imgDelegate.modelRotation
                    
                    itemIndex: imgDelegate.index
                    
                    //привязываем свойство компонента ImageItem.selected к роли модели
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

    //рамка выделения поверх Flickable
    Rectangle {
        id: selectionRect
        visible: false
        color: Qt.rgba(
            ThemeManager.accentColor.r, ThemeManager.accentColor.g, ThemeManager.accentColor.b, 0.15
        )
        border.color: ThemeManager.accentColor
        border.width: 1
        z: 1000
    }

    //единая MouseArea для выделения и панорамирования
    //z установлено в -1, чтобы элементы управления могли перехватывать события
    MouseArea {
        id: mainMouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        hoverEnabled: true
        z: -1

        property point pressPos //позиция нажатия в координатах экрана
        property point lastPos //последняя позиция (для панорамирования)
        property bool isPanning: false
        property bool isSelecting: false
        property bool isDragging: false //перетаскивание выделенного изображения
        property point dragStartScene //начальная позиция мыши в сцене при начале drag

        onPressed: function(mouse) {
            if (mouse.button === Qt.MiddleButton) {
                //панорамирование средней кнопкой
                isPanning = true
                lastPos = Qt.point(mouse.x, mouse.y)
                cursorShape = Qt.ClosedHandCursor
                return
            }

            //ЛКМ — определяем, что под курсором
            pressPos = Qt.point(mouse.x, mouse.y)
            var scenePos = mapToScene(pressPos)
            var hitIdx = controller.selectionController.hitTest(scenePos.x, scenePos.y)

            //клик по изображению
            if (hitIdx >= 0) {
                if (resizeMode || cropMode) {
                    //в режиме инструмента — всегда переключаем выделение на кликнутый элемент
                    controller.selectionController.selectItem(hitIdx, false)
                }
                else if (mouse.modifiers & Qt.ControlModifier) {
                    //ctrl — убрать из выделения
                    controller.selectionController.deselectItem(hitIdx)
                }
                else if (mouse.modifiers & Qt.ShiftModifier) {
                    //shift — добавить к выделению
                    controller.selectionController.selectItem(hitIdx, true)
                }
                else {
                    //обычный клик — единственное выделение (если не уже выделен)
                    if (!controller.selectionController.getIsItemSelected(hitIdx)) {
                        controller.selectionController.selectItem(hitIdx, false)
                    }
                }

                //drag только если инструменты не активны
                if (!resizeMode && !cropMode) {
                    isDragging = true
                    dragStartScene = scenePos
                    controller.beginMoveSelection()
                    cursorShape = Qt.ClosedHandCursor
                }
            }
            //клик по пустому месту начинаем рамочное выделение
            else {
                //деактивируем режимы инструментов
                resizeMode = false
                cropMode = false

                isSelecting = true
                selectionRect.x = mouse.x
                selectionRect.y = mouse.y
                selectionRect.width = 0
                selectionRect.height = 0
                selectionRect.visible = true

                //очищаем выделение если не зажат shift
                if (!(mouse.modifiers & Qt.ShiftModifier)) {
                    controller.selectionController.clearSelection()
                }
            }
        }

        onPositionChanged: function(mouse) {
            if (isPanning) {
                //панорамирование
                var dx = mouse.x - lastPos.x
                var dy = mouse.y - lastPos.y
                flickable.contentX -= dx
                flickable.contentY -= dy
                lastPos = Qt.point(mouse.x, mouse.y)
            }
            else if (isDragging) {
                //перемещение элементов — дельта от начальной точки мыши
                var currentScene = mapToScene(Qt.point(mouse.x, mouse.y))
                var deltaX = currentScene.x - dragStartScene.x
                var deltaY = currentScene.y - dragStartScene.y
                
                controller.updateMoveSelection(deltaX, deltaY)
                cursorShape = Qt.ClosedHandCursor
            }
            else if (isSelecting) {
                //обновляем рамку выделения
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
            }
            else if (isDragging) {
                isDragging = false
                cursorShape = Qt.ArrowCursor

                // Создаём undo-команду для выделения (сравнивает старые и новые координаты внутри C++)
                controller.endMoveSelection()
            }
            else if (isSelecting) {
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

                    controller.selectionController.selectInRect(rectX, rectY, rectW, rectH,
                                            mouse.modifiers & Qt.ShiftModifier)
                }
                else if (selectionRect.width < 3 && selectionRect.height < 3) {
                    // Мини-клик по пустому месту — очищаем выделение
                    if (!(mouse.modifiers & Qt.ShiftModifier)) {
                        controller.selectionController.clearSelection()
                    }
                }
            }
        }
    }

    //обработка колёсика мыши для зума и прокрутки
    MouseArea {
        id: wheelArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton

        onWheel: function(wheel) {
            if (wheel.modifiers & Qt.ControlModifier) {
                //зум с Ctrl
                var factor = wheel.angleDelta.y > 0 ? 1.15 : 1/1.15
                var newZoom = Math.max(minZoom, Math.min(maxZoom, zoomLevel * factor))
                setZoom(newZoom, Qt.point(wheel.x, wheel.y))
            }
            else {
                //прокрутка без Ctrl
                flickable.contentX -= wheel.angleDelta.x
                flickable.contentY -= wheel.angleDelta.y
            }
        }
    }

    //drag & drop файлов
    DropArea {
        anchors.fill: parent

        onDropped: function(drop) {
            if (drop.hasUrls) {
                for (var i = 0; i < drop.urls.length; i++) {
                    var scenePos = mapToScene(Qt.point(drop.x, drop.y))
                    controller.clipboardController.addImage(drop.urls[i], scenePos.x, scenePos.y)
                }
            }
        }
    }
}
