import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

/**
 * WelcomeWindow.qml - стартовое окно приложения.
 * Позволяет создать новую доску или открыть существующую.
 */
Dialog {
    id: root
    
    width: 900
    height: 600
    modal: true
    closePolicy: Dialog.NoAutoClose
    
    signal newBoardRequested()
    signal openBoardRequested(url fileUrl)
    
    background: Item {
        // Паттерн фона
        Image {
            anchors.fill: parent
            source: "qrc:/graphics/graphics/pattern.png"
            fillMode: Image.Tile
            sourceSize: Qt.size(400, 400)
        }
    }
    
    // Основной контент
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 50
        spacing: 30
        
        // Логотип
        Image {
            id: logoImage
            source: "qrc:/graphics/graphics/text-logo.png"
            Layout.alignment: Qt.AlignHCenter
            sourceSize.height: 160
            fillMode: Image.PreserveAspectFit
        }
        
        Item { height: 30 }
        
        // Заголовок "Recent projects"
        Label {
            text: "Recent projects"
            font.pixelSize: 26
            font.bold: true
            color: "#141414"
            background: Item {}
        }
        
        // Заглушки для последних проектов
        RowLayout {
            spacing: 15
            Layout.fillWidth: true
            
            Repeater {
                model: 5
                
                Rectangle {
                    width: 150
                    height: 100
                    color: Qt.rgba(0, 0, 0, 0.2)
                    border.color: Qt.rgba(0, 0, 0, 0.3)
                    border.width: 1
                    radius: 6
                }
            }
            
            Item { Layout.fillWidth: true }
        }
        
        // Растягивающийся элемент
        Item { Layout.fillHeight: true }
        
        // Кнопки внизу
        RowLayout {
            Layout.fillWidth: true
            spacing: 15
            
            Item { Layout.fillWidth: true }
            
            Button {
                id: newButton
                text: "Создать"
                font.pixelSize: 18
                implicitHeight: 50
                implicitWidth: 120
                
                background: Rectangle {
                    color: newButton.pressed ? Theme.accentPressedColor : 
                           newButton.hovered ? Theme.accentHoverColor : Theme.accentColor
                    radius: 5
                }
                
                contentItem: Text {
                    text: newButton.text
                    font: newButton.font
                    color: newButton.pressed ? "white" : Theme.backgroundColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: root.newBoardRequested()
            }
            
            Button {
                id: openButton
                text: "Открыть"
                font.pixelSize: 18
                implicitHeight: 50
                implicitWidth: 120
                
                background: Rectangle {
                    color: openButton.pressed ? Theme.accentPressedColor : 
                           openButton.hovered ? Theme.accentHoverColor : Theme.accentColor
                    radius: 5
                }
                
                contentItem: Text {
                    text: openButton.text
                    font: openButton.font
                    color: openButton.pressed ? "white" : Theme.backgroundColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: fileDialog.open()
            }
        }
    }
    
    // Диалог выбора файла
    FileDialog {
        id: fileDialog
        title: "Открыть доску"
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: root.openBoardRequested(selectedFile)
    }
}
