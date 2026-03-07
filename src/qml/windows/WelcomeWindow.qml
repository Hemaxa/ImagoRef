//WelcomeWindow.qml - стартовое окно приложения

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root
    
    width: 600
    height: 800
    modal: true
    closePolicy: Dialog.NoAutoClose
    
    signal newBoardRequested()
    signal openBoardRequested(url fileUrl)
    
    // Цвета темы Welcome Window
    property color bgColor: ThemeManager.welcomeBgColor
    property color btnNewGradientStart: ThemeManager.welcomeBtnNewGradientStart
    property color btnNewGradientEnd: ThemeManager.welcomeBtnNewGradientEnd
    property color btnOpenColor: ThemeManager.welcomeBtnOpenColor
    property color textDark: ThemeManager.welcomeTextDark
    property color accentYellow: ThemeManager.welcomeAccentYellow
    
    // ========================================
    // BACKGROUND
    // ========================================
    background: Rectangle {
        color: root.bgColor
        
        // Декоративные элементы фона
        Image {
            source: ThemeManager.welcomeDecoTrianglePath
            x: 50; y: 80
            width: 40; height: 40
            rotation: -15
        }
        
        // Зигзаг справа
        Image {
            source: ThemeManager.welcomeDecoZigzagPath
            x: parent.width - 100; y: 150
            width: 60; height: 25
        }
        
        // Звезда слева
        Image {
            source: ThemeManager.welcomeDecoStarPath
            x: 30; y: 250
            width: 25; height: 25
        }
        
        // Точки справа вверху
        Image {
            source: ThemeManager.welcomeDecoDotsPath
            x: parent.width - 80; y: 60
            width: 50; height: 50
        }
        
        // Еще элементы для заполнения
        Image {
            source: ThemeManager.welcomeDecoTrianglePath
            x: parent.width - 150; y: 400
            width: 35; height: 35
            rotation: 45
        }
        
        Image {
            source: ThemeManager.welcomeDecoZigzagPath
            x: 80; y: parent.height - 150
            width: 50; height: 20
            rotation: -30
        }
        
        Image {
            source: ThemeManager.welcomeDecoStarPath
            x: parent.width - 60; y: parent.height - 200
            width: 20; height: 20
        }
    }
    
    // ========================================
    // MAIN CONTENT
    // ========================================
    Item {
        anchors.fill: parent
        
        // Логотип вверху по центру
        Image {
            id: logoImage
            source: ThemeManager.logoPath
            anchors.horizontalCenter: parent.horizontalCenter
            y: 40
            width: 550
            height: 120
            fillMode: Image.PreserveAspectFit
        }
        
        // Заголовок "Recent projects"
        Item {
            id: sectionHeader
            anchors.top: logoImage.bottom
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 40
            width: 200
            height: 40
            
            // Желтая линия-подчеркивание
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 20
                color: root.accentYellow
            }

            Text {
                text: "Recent projects"
                font.pixelSize: 30
                font.bold: true
                color: root.textDark
            }
        }
        
        // Сетка проектов 2×3
        GridLayout {
            id: projectGrid
            anchors.top: sectionHeader.bottom
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
            
            columns: 3
            rowSpacing: 20
            columnSpacing: 20
            
            Repeater {
                model: 6
                
                // Карточка проекта
                Item {
                    width: 160
                    height: 160
                    
                    // Рамка с полосками
                    Image {
                        id: frameImage
                        anchors.fill: parent
                        source: ThemeManager.projectFramePath
                        fillMode: Image.Stretch
                    }
                    
                    // Превью проекта (пока заглушка)
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 20
                        height: parent.height - 20
                        color: ThemeManager.controlBackground
                        
                        Text {
                            anchors.centerIn: parent
                            text: "Project " + (index + 1)
                            color: ThemeManager.textColor
                            font.pixelSize: 12
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            // TODO: открыть проект из истории
                            console.log("Open recent project", index)
                        }
                    }
                }
            }
        }
        
        // ========================================
        // MASCOT (персонаж слева внизу)
        // ========================================
        Image {
            id: mascotImage
            source: ThemeManager.mascotPath
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: 280
            height: 380
            fillMode: Image.PreserveAspectFit
        }
        
        // ========================================
        // BUTTONS (справа внизу)
        // ========================================
        Row {
            id: buttonsRow
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 50
            anchors.bottomMargin: 50
            spacing: 20
            
            // Кнопка "New Board"
            Rectangle {
                id: newBoardBtn
                width: 160
                height: 50
                radius: 25
                
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: root.btnNewGradientStart }
                    GradientStop { position: 1.0; color: root.btnNewGradientEnd }
                }
                
                Text {
                    anchors.centerIn: parent
                    text: "New Board"
                    font.pixelSize: 18
                    font.bold: true
                    color: root.textDark
                }
                
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    
                    onClicked: root.newBoardRequested()
                    
                    onEntered: parent.scale = 1.05
                    onExited: parent.scale = 1.0
                }
                
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }
            
            // Кнопка "Open Existing"
            Rectangle {
                id: openExistingBtn
                width: 180
                height: 50
                radius: 25
                color: root.btnOpenColor
                
                // Клетчатый паттерн (эффект)
                Canvas {
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)
                        
                        // Рисуем клетки
                        var cellSize = 10
                        ctx.fillStyle = Qt.rgba(0, 0, 0, 0.15)
                        
                        for (var y = 0; y < height; y += cellSize * 2) {
                            for (var x = 0; x < width; x += cellSize * 2) {
                                ctx.fillRect(x, y, cellSize, cellSize)
                                ctx.fillRect(x + cellSize, y + cellSize, cellSize, cellSize)
                            }
                        }
                    }
                }
                
                Text {
                    anchors.centerIn: parent
                    text: "Open Existing"
                    font.pixelSize: 18
                    font.bold: true
                    color: root.textDark
                }
                
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    
                    onClicked: fileDialog.open()
                    
                    onEntered: parent.scale = 1.05
                    onExited: parent.scale = 1.0
                }
                
                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }
            }
        }
    }
    
    // ========================================
    // FILE DIALOG
    // ========================================
    FileDialog {
        id: fileDialog
        title: "Открыть доску"
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: root.openBoardRequested(selectedFile)
    }
}
