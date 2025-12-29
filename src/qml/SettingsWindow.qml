import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * SettingsWindow.qml - диалог настроек приложения.
 */
Dialog {
    id: root
    
    title: "Настройки"
    width: 750
    height: 450
    modal: true
    
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    background: Rectangle {
        color: Theme.backgroundColor
        border.color: Theme.borderColor
        border.width: 1
        radius: 8
    }
    
    onAccepted: {
        Settings.gridSize = gridSizeSpinBox.value
        Settings.themeName = themeComboBox.currentValue
        Theme.applyTheme(themeComboBox.currentValue)
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Навигационный список
        ListView {
            id: navigationList
            Layout.preferredWidth: 180
            Layout.fillHeight: true
            currentIndex: 0
            
            model: ListModel {
                ListElement { name: "Общие" }
                ListElement { name: "Горячие клавиши" }
            }
            
            delegate: ItemDelegate {
                width: navigationList.width
                height: 44
                
                background: Rectangle {
                    color: navigationList.currentIndex === index ? Theme.accentColor : "transparent"
                    radius: 10
                }
                
                contentItem: Text {
                    text: model.name
                    color: navigationList.currentIndex === index ? Theme.backgroundColor : Theme.textColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 10
                }
                
                onClicked: navigationList.currentIndex = index
            }
            
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: Theme.borderColor
                border.width: 1
                radius: 12
                z: -1
            }
        }
        
        // Страницы настроек
        StackLayout {
            currentIndex: navigationList.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            // Страница "Общие"
            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20
                    
                    // Шаг сетки
                    RowLayout {
                        spacing: 30
                        
                        Label {
                            text: "Шаг сетки:"
                            color: Theme.textColor
                        }
                        
                        SpinBox {
                            id: gridSizeSpinBox
                            from: 20
                            to: 200
                            value: Settings.gridSize
                            editable: true
                            
                            textFromValue: function(value) {
                                return value + " px"
                            }
                            
                            valueFromText: function(text) {
                                return parseInt(text.replace(" px", ""))
                            }
                        }
                    }
                    
                    // Тема интерфейса
                    RowLayout {
                        spacing: 30
                        
                        Label {
                            text: "Тема интерфейса:"
                            color: Theme.textColor
                        }
                        
                        ComboBox {
                            id: themeComboBox
                            Layout.preferredWidth: 200
                            
                            model: ListModel {
                                ListElement { text: "Imago"; value: "imago" }
                                ListElement { text: "Темная"; value: "dark" }
                                ListElement { text: "Светлая"; value: "light" }
                                ListElement { text: "Голубая"; value: "blue" }
                                ListElement { text: "Аквамариновая"; value: "aquamarine" }
                                ListElement { text: "Зеленая"; value: "green" }
                                ListElement { text: "Фиолетовая"; value: "purple" }
                                ListElement { text: "Розовая"; value: "pink" }
                                ListElement { text: "Оранжевая"; value: "orange" }
                            }
                            
                            textRole: "text"
                            valueRole: "value"
                            
                            Component.onCompleted: {
                                currentIndex = indexOfValue(Settings.themeName)
                            }
                        }
                    }
                    
                    Item { Layout.fillHeight: true }
                }
            }
            
            // Страница "Горячие клавиши"
            ScrollView {
                clip: true
                
                ColumnLayout {
                    width: parent.width
                    spacing: 15
                    
                    // Навигация и холст
                    Label {
                        text: "Навигация и холст"
                        font.bold: true
                        font.pixelSize: 15
                        color: Theme.textColor
                        
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: Theme.accentColor
                        }
                    }
                    
                    HotkeyRow { description: "Перемещение холста:"; keys: "Зажать среднюю кнопку мыши" }
                    HotkeyRow { description: "Масштабирование:"; keys: "Ctrl/Cmd + Колесико мыши" }
                    HotkeyRow { description: "Приблизить:"; keys: "Ctrl/Cmd + Плюс (+)" }
                    HotkeyRow { description: "Отдалить:"; keys: "Ctrl/Cmd + Минус (-)" }
                    HotkeyRow { description: "Привязать все к сетке:"; keys: "Ctrl + G" }
                    
                    Item { height: 10 }
                    
                    // Управление элементами
                    Label {
                        text: "Управление элементами"
                        font.bold: true
                        font.pixelSize: 15
                        color: Theme.textColor
                        
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: Theme.accentColor
                        }
                    }
                    
                    HotkeyRow { description: "Вставить из буфера:"; keys: "Ctrl/Cmd + V" }
                    HotkeyRow { description: "Удалить выделенное:"; keys: "Delete/Backspace" }
                    HotkeyRow { description: "Отменить действие:"; keys: "Ctrl/Cmd + Z" }
                    HotkeyRow { description: "Повторить действие:"; keys: "Ctrl/Cmd + Shift + Z" }
                    
                    Item { height: 10 }
                    
                    // Трансформации
                    Label {
                        text: "Трансформации"
                        font.bold: true
                        font.pixelSize: 15
                        color: Theme.textColor
                        
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: Theme.accentColor
                        }
                    }
                    
                    HotkeyRow { description: "Режим изменения размера:"; keys: "Ctrl + E" }
                    HotkeyRow { description: "Вращать по часовой:"; keys: "Ctrl + R" }
                    HotkeyRow { description: "Вращать против часовой:"; keys: "Ctrl + Shift + R" }
                    HotkeyRow { description: "Сохранять пропорции:"; keys: "Зажать Shift" }
                    
                    Item { height: 10 }
                    
                    // Интерфейс
                    Label {
                        text: "Интерфейс"
                        font.bold: true
                        font.pixelSize: 15
                        color: Theme.textColor
                        
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: 1
                            color: Theme.accentColor
                        }
                    }
                    
                    HotkeyRow { description: "Скрыть/показать панель:"; keys: "Tab" }
                    HotkeyRow { description: "Открыть настройки:"; keys: "Ctrl/Cmd + ," }
                    
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
    
    // Компонент для строки горячей клавиши
    component HotkeyRow: RowLayout {
        property string description
        property string keys
        
        spacing: 20
        
        Label {
            text: description
            color: Theme.textColor
        }
        
        Label {
            text: keys
            font.bold: true
            color: Theme.textColor
        }
    }
}
