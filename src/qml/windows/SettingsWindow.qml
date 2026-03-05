import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ImagoRef

/**
 * SettingsWindow.qml - диалог настроек приложения.
 * Оформлен в стиле основного приложения (тёмный фон, акцентные элементы).
 */
Dialog {
    id: root
    
    title: ""
    width: 700
    height: 420
    modal: true
    
    standardButtons: Dialog.NoButton
    
    background: Rectangle {
        color: ThemeManager.backgroundColor
        border.color: ThemeManager.accentColor
        border.width: 2
        radius: 12
    }
    
    // Пользовательский заголовок
    header: Item {
        height: 48
        
        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(ThemeManager.accentColor.r, ThemeManager.accentColor.g, ThemeManager.accentColor.b, 0.15)
            radius: 12
            
            // Скругление только сверху
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 12
                color: parent.color
            }
        }
        
        Text {
            anchors.centerIn: parent
            text: "Настройки"
            font.pixelSize: 16
            font.bold: true
            color: ThemeManager.accentColor
        }
    }
    
    onAccepted: {
        SettingsManager.gridSize = gridSizeSpinBox.value
        SettingsManager.themeName = themeComboBox.currentValue
        SettingsManager.canvasPattern = patternComboBox.currentValue
        SettingsManager.labelFontSize = labelFontSizeSpinBox.value
        SettingsManager.arrangeSpacing = arrangeSpacingSpinBox.value
        ThemeManager.applyTheme(themeComboBox.currentValue)
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Навигационный список
        ListView {
            id: navigationList
            Layout.preferredWidth: 160
            Layout.fillHeight: true
            currentIndex: 0
            
            model: ListModel {
                ListElement { name: "Общие" }
                ListElement { name: "Горячие клавиши" }
            }
            
            delegate: ItemDelegate {
                width: navigationList.width
                height: 42
                
                background: Rectangle {
                    color: navigationList.currentIndex === index 
                           ? Qt.rgba(ThemeManager.accentColor.r, ThemeManager.accentColor.g, ThemeManager.accentColor.b, 0.25)
                           : (hovered ? Qt.rgba(ThemeManager.accentColor.r, ThemeManager.accentColor.g, ThemeManager.accentColor.b, 0.1) : "transparent")
                    radius: 8
                    border.color: navigationList.currentIndex === index ? ThemeManager.accentColor : "transparent"
                    border.width: 1
                }
                
                contentItem: Text {
                    text: model.name
                    color: navigationList.currentIndex === index ? ThemeManager.accentColor : ThemeManager.textColor
                    font.bold: navigationList.currentIndex === index
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: 12
                }
                
                onClicked: navigationList.currentIndex = index
            }
            
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: ThemeManager.borderColor
                border.width: 1
                radius: 8
                z: -1
            }
        }
        
        // Разделитель
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: ThemeManager.borderColor
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
                    anchors.margins: 15
                    spacing: 18
                    
                    // Шаг сетки
                    RowLayout {
                        spacing: 0
                        
                        Label {
                            text: "Шаг сетки"
                            color: ThemeManager.textColor
                            Layout.preferredWidth: 160
                        }
                        
                        SpinBox {
                            id: gridSizeSpinBox
                            from: 20
                            to: 200
                            value: SettingsManager.gridSize
                            editable: true
                            Layout.preferredWidth: 180
                            
                            textFromValue: function(value) {
                                return value + " px"
                            }
                            
                            valueFromText: function(text) {
                                return parseInt(text.replace(" px", ""))
                            }
                            
                            background: Rectangle {
                                color: ThemeManager.controlBackground
                                border.color: ThemeManager.borderColor
                                border.width: 1
                                radius: 6
                            }
                            
                            contentItem: TextInput {
                                text: gridSizeSpinBox.textFromValue(gridSizeSpinBox.value, gridSizeSpinBox.locale)
                                font.pixelSize: 13
                                color: ThemeManager.textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                readOnly: !gridSizeSpinBox.editable
                                validator: gridSizeSpinBox.validator
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                        }
                    }
                    
                    // Тема интерфейса
                    RowLayout {
                        spacing: 0
                        
                        Label {
                            text: "Тема интерфейса"
                            color: ThemeManager.textColor
                            Layout.preferredWidth: 160
                        }
                        
                        ComboBox {
                            id: themeComboBox
                            Layout.preferredWidth: 180
                            
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
                            
                            background: Rectangle {
                                color: ThemeManager.controlBackground
                                border.color: ThemeManager.borderColor
                                border.width: 1
                                radius: 6
                            }
                            
                            contentItem: Text {
                                text: themeComboBox.displayText
                                font.pixelSize: 13
                                color: ThemeManager.textColor
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }
                            
                            Component.onCompleted: {
                                currentIndex = indexOfValue(SettingsManager.themeName)
                            }
                        }
                    }
                    
                    // Паттерн рабочей области
                    RowLayout {
                        spacing: 0
                        
                        Label {
                            text: "Паттерн холста"
                            color: ThemeManager.textColor
                            Layout.preferredWidth: 160
                        }
                        
                        ComboBox {
                            id: patternComboBox
                            Layout.preferredWidth: 180
                            
                            model: ListModel {
                                ListElement { text: "Точки"; value: "dots" }
                                ListElement { text: "Крестики"; value: "cross" }
                                ListElement { text: "Без паттерна"; value: "none" }
                            }
                            
                            textRole: "text"
                            valueRole: "value"
                            
                            background: Rectangle {
                                color: ThemeManager.controlBackground
                                border.color: ThemeManager.borderColor
                                border.width: 1
                                radius: 6
                            }
                            
                            contentItem: Text {
                                text: patternComboBox.displayText
                                font.pixelSize: 13
                                color: ThemeManager.textColor
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }
                            
                            Component.onCompleted: {
                                currentIndex = indexOfValue(SettingsManager.canvasPattern)
                            }
                        }
                    }
                    
                    // Размер шрифта подписи
                    RowLayout {
                        spacing: 0
                        
                        Label {
                            text: "Размер шрифта подписи"
                            color: ThemeManager.textColor
                            Layout.preferredWidth: 160
                        }
                        
                        SpinBox {
                            id: labelFontSizeSpinBox
                            from: 8
                            to: 72
                            value: SettingsManager.labelFontSize
                            editable: true
                            Layout.preferredWidth: 180
                            
                            textFromValue: function(value) {
                                return value + " px"
                            }
                            
                            valueFromText: function(text) {
                                return parseInt(text.replace(" px", ""))
                            }
                            
                            background: Rectangle {
                                color: ThemeManager.controlBackground
                                border.color: ThemeManager.borderColor
                                border.width: 1
                                radius: 6
                            }
                            
                            contentItem: TextInput {
                                text: labelFontSizeSpinBox.textFromValue(labelFontSizeSpinBox.value, labelFontSizeSpinBox.locale)
                                font.pixelSize: 13
                                color: ThemeManager.textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                readOnly: !labelFontSizeSpinBox.editable
                                validator: labelFontSizeSpinBox.validator
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                        }
                    }
                    
                    // Отступ при расположении
                    RowLayout {
                        spacing: 0
                        
                        Label {
                            text: "Отступ при расположении"
                            color: ThemeManager.textColor
                            Layout.preferredWidth: 160
                        }
                        
                        SpinBox {
                            id: arrangeSpacingSpinBox
                            from: 0
                            to: 200
                            value: SettingsManager.arrangeSpacing
                            editable: true
                            Layout.preferredWidth: 180
                            
                            textFromValue: function(value) {
                                return value + " px"
                            }
                            
                            valueFromText: function(text) {
                                return parseInt(text.replace(" px", ""))
                            }
                            
                            background: Rectangle {
                                color: ThemeManager.controlBackground
                                border.color: ThemeManager.borderColor
                                border.width: 1
                                radius: 6
                            }
                            
                            contentItem: TextInput {
                                text: arrangeSpacingSpinBox.textFromValue(arrangeSpacingSpinBox.value, arrangeSpacingSpinBox.locale)
                                font.pixelSize: 13
                                color: ThemeManager.textColor
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                readOnly: !arrangeSpacingSpinBox.editable
                                validator: arrangeSpacingSpinBox.validator
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
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
                    spacing: 10
                    
                    // Навигация и холст
                    Label {
                        text: "Навигация и холст"
                        font.bold: true
                        font.pixelSize: 14
                        color: ThemeManager.accentColor
                        bottomPadding: 2
                    }
                    
                    HotkeyRow { description: "Перемещение холста:"; keys: "Зажать ⌥ / среднюю кнопку мыши" }
                    HotkeyRow { description: "Масштабирование:"; keys: "⌘/Ctrl + Колесико мыши" }
                    HotkeyRow { description: "Приблизить:"; keys: "⌘/Ctrl + +" }
                    HotkeyRow { description: "Отдалить:"; keys: "⌘/Ctrl + -" }
                    HotkeyRow { description: "Привязать к сетке:"; keys: "G" }
                    
                    Item { height: 8 }
                    
                    // Управление элементами
                    Label {
                        text: "Управление элементами"
                        font.bold: true
                        font.pixelSize: 14
                        color: ThemeManager.accentColor
                        bottomPadding: 2
                    }
                    
                    HotkeyRow { description: "Выделить все:"; keys: "⌘/Ctrl + A" }
                    HotkeyRow { description: "Вставить из буфера:"; keys: "⌘/Ctrl + V" }
                    HotkeyRow { description: "Удалить выделенное:"; keys: "Delete / Backspace" }
                    HotkeyRow { description: "Отменить действие:"; keys: "⌘/Ctrl + Z" }
                    HotkeyRow { description: "Повторить действие:"; keys: "⌘/Ctrl + Shift + Z" }
                    
                    Item { height: 8 }
                    
                    // Трансформации
                    Label {
                        text: "Трансформации"
                        font.bold: true
                        font.pixelSize: 14
                        color: ThemeManager.accentColor
                        bottomPadding: 2
                    }
                    
                    HotkeyRow { description: "Изменить размер:"; keys: "S" }
                    HotkeyRow { description: "Обрезать:"; keys: "C" }
                    HotkeyRow { description: "Вращать по часовой:"; keys: "R" }
                    HotkeyRow { description: "Вращать против часовой:"; keys: "Shift + R" }
                    HotkeyRow { description: "Сохранять пропорции:"; keys: "Зажать Shift" }
                    
                    Item { height: 8 }
                    
                    // Интерфейс
                    Label {
                        text: "Интерфейс"
                        font.bold: true
                        font.pixelSize: 14
                        color: ThemeManager.accentColor
                        bottomPadding: 2
                    }
                    
                    HotkeyRow { description: "Скрыть/показать панель:"; keys: "Tab" }
                    HotkeyRow { description: "Открыть настройки:"; keys: "⌘/Ctrl + ," }
                    HotkeyRow { description: "Сбросить инструменты:"; keys: "Escape" }
                    
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
    
    // Кнопки OK / Отмена
    footer: Item {
        height: 52
        
        RowLayout {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 15
            spacing: 10
            
            // Отмена
            AbstractButton {
                id: cancelBtn
                Layout.preferredWidth: 90
                Layout.preferredHeight: 32
                
                background: Rectangle {
                    color: cancelBtn.hovered ? Qt.rgba(ThemeManager.textColor.r, ThemeManager.textColor.g, ThemeManager.textColor.b, 0.1) : "transparent"
                    border.color: ThemeManager.borderColor
                    border.width: 1
                    radius: 6
                }
                
                contentItem: Text {
                    text: "Отмена"
                    color: ThemeManager.textColor
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: root.reject()
            }
            
            // OK
            AbstractButton {
                id: applyBtn
                Layout.preferredWidth: 90
                Layout.preferredHeight: 32
                
                background: Rectangle {
                    color: applyBtn.hovered ? ThemeManager.accentPressedColor : ThemeManager.accentColor
                    radius: 6
                }
                
                contentItem: Text {
                    text: "Применить"
                    color: ThemeManager.backgroundColor
                    font.pixelSize: 13
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: root.accept()
            }
        }
    }
    
    // Компонент для строки горячей клавиши
    component HotkeyRow: RowLayout {
        property string description
        property string keys
        
        spacing: 0
        
        Label {
            text: description
            color: Qt.rgba(ThemeManager.textColor.r, ThemeManager.textColor.g, ThemeManager.textColor.b, 0.7)
            font.pixelSize: 12
            Layout.preferredWidth: 200
        }
        
        Label {
            text: keys
            font.pixelSize: 12
            font.bold: true
            color: ThemeManager.textColor
        }
    }
}
