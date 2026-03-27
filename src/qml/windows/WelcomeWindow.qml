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
    signal openCloudBoardRequested(string boardId)
    signal newCloudBoardRequested() // Сигнал для MainWindow, чтобы открыть пустую доску и привязать к Cloud

    // Цвета темы Welcome Window
    property color bgColor: ThemeManager.colors.welcomeBgColor
    property color btnNewGradientStart: ThemeManager.colors.welcomeBtnNewGradientStart
    property color btnNewGradientEnd: ThemeManager.colors.welcomeBtnNewGradientEnd
    property color btnOpenColor: ThemeManager.colors.welcomeBtnOpenColor
    property color textDark: ThemeManager.colors.welcomeTextDark
    property color accentYellow: ThemeManager.colors.welcomeAccentYellow
    
    // ========================================
    // BACKGROUND
    // ========================================
    background: Rectangle {
        color: root.bgColor
        
        // Декоративные элементы фона
        Image {
            source: ThemeManager.icons.welcomeDecoTriangle
            x: 50; y: 80
            width: 40; height: 40
            rotation: -15
        }
        
        // Зигзаг справа
        Image {
            source: ThemeManager.icons.welcomeDecoZigzag
            x: parent.width - 100; y: 150
            width: 60; height: 25
        }
        
        // Звезда слева
        Image {
            source: ThemeManager.icons.welcomeDecoStar
            x: 30; y: 250
            width: 25; height: 25
        }
        
        // Точки справа вверху
        Image {
            source: ThemeManager.icons.welcomeDecoDots
            x: parent.width - 80; y: 60
            width: 50; height: 50
        }
        
        // Еще элементы для заполнения
        Image {
            source: ThemeManager.icons.welcomeDecoTriangle
            x: parent.width - 150; y: 400
            width: 35; height: 35
            rotation: 45
        }
        
        Image {
            source: ThemeManager.icons.welcomeDecoZigzag
            x: 80; y: parent.height - 150
            width: 50; height: 20
            rotation: -30
        }
        
        Image {
            source: ThemeManager.icons.welcomeDecoStar
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
            source: ThemeManager.icons.logo
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

        // Кнопка авторизации / Аватар пользователя (В правом верхнем углу)
        Item {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 30
            anchors.rightMargin: 30
            width: AuthController.isLoggedIn ? 50 : 120
            height: 50

            Rectangle {
                anchors.fill: parent
                radius: 25
                color: AuthController.isLoggedIn ? ThemeManager.colors.controlBackground : root.accentYellow
                border.color: ThemeManager.colors.controlBorder
                border.width: 1

                Text {
                    id: authBtnText
                    anchors.centerIn: parent
                    text: AuthController.isLoggedIn ? AuthController.userEmail.charAt(0).toUpperCase() : "Войти"
                    font.pixelSize: AuthController.isLoggedIn ? 24 : 18
                    font.bold: true
                    color: AuthController.isLoggedIn ? ThemeManager.colors.textColor : root.textDark
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        if (AuthController.isLoggedIn) {
                            avatarMenu.open()
                        } else {
                            authDialog.open()
                        }
                    }

                    onEntered: parent.scale = 1.05
                    onExited: parent.scale = 1.0
                }
                
                Behavior on scale { NumberAnimation { duration: 100 } }
                
                Menu {
                    id: avatarMenu
                    y: parent.height + 5
                    MenuItem {
                        text: "Доски"
                        onTriggered: {
                            CloudBoardsManager.fetchBoards()
                            cloudDashboardDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Выйти"
                        onTriggered: AuthController.logout()
                    }
                }
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
                model: SettingsManager.recentBoards.length === 0 ? 0 : Math.min(6, SettingsManager.recentBoards.length)
                
                // Карточка проекта
                Item {
                    width: 160
                    height: 160
                    
                    // Рамка с полосками
                    Image {
                        id: frameImage
                        anchors.fill: parent
                        source: ThemeManager.icons.projectFrame
                        fillMode: Image.Stretch
                    }
                    
                    // Превью проекта
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width - 20
                        height: parent.height - 20
                        color: ThemeManager.colors.controlBackground
                        
                        // Если есть превью, можно показать картинку. Пока текст + иконка типа
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 10
                            
                            Image {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                source: modelData.type === "cloud" ? ThemeManager.icons.sidebarTabCloud : ThemeManager.icons.sidebarTabLocal
                                // Замечание: используем иконки, нужно убедиться что они есть, или подставим плейсхолдеры.
                                // Для облака используем что-то, для локалки что-то.
                            }
                            
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: modelData.name || "Untitled"
                                color: ThemeManager.colors.textColor
                                font.pixelSize: 14
                                width: 120
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (modelData.type === "cloud") {
                                root.openCloudBoardRequested(modelData.id)
                            } else {
                                root.openBoardRequested(modelData.path)
                            }
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
            source: ThemeManager.icons.mascot
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: 280
            height: 380
            fillMode: Image.PreserveAspectFit
            verticalAlignment: Image.AlignBottom
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
                    
                    onClicked: {
                        if (AuthController.isLoggedIn) {
                            CloudBoardsManager.createBoard("New Board")
                        } else {
                            root.newBoardRequested()
                        }
                    }
                    
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
                    
                    onClicked: {
                        if (AuthController.isLoggedIn) {
                            openChoiceMenu.open()
                        } else {
                            fileDialog.open()
                        }
                    }
                    
                    Menu {
                        id: openChoiceMenu
                        y: -height - 10
                        MenuItem {
                            text: "Открыть локальный файл"
                            onTriggered: fileDialog.open()
                        }
                        MenuItem {
                            text: "Открыть из облака"
                            onTriggered: {
                                CloudBoardsManager.fetchBoards()
                                cloudDashboardDialog.open()
                            }
                        }
                    }
                    
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

    // ========================================
    // AUTH DIALOG
    // ========================================
    Dialog {
        id: authDialog
        title: "Авторизация"
        width: 350
        anchors.centerIn: parent
        modal: true
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 15
            
            TabBar {
                id: authTabBar
                Layout.fillWidth: true
                TabButton { text: "Вход" }
                TabButton { text: "Регистрация" }
            }
            
            TextField {
                id: emailInput
                Layout.fillWidth: true
                placeholderText: "Email"
            }
            
            TextField {
                id: passwordInput
                Layout.fillWidth: true
                placeholderText: "Пароль"
                echoMode: TextInput.Password
            }
            
            Text {
                id: authErrorText
                Layout.fillWidth: true
                color: "red"
                wrapMode: Text.Wrap
                visible: text !== ""
            }
            
            Button {
                Layout.fillWidth: true
                text: authTabBar.currentIndex === 0 ? "Войти" : "Зарегистрироваться"
                onClicked: {
                    authErrorText.text = ""
                    if (authTabBar.currentIndex === 0) {
                        AuthController.login(emailInput.text, passwordInput.text)
                    } else {
                        AuthController.registerUser(emailInput.text, passwordInput.text)
                    }
                }
            }
        }
        
        Connections {
            target: AuthController
            function onLoginFinished(success, message) {
                if (success) {
                    authDialog.close()
                } else {
                    authErrorText.text = message
                }
            }
            function onRegisterFinished(success, message) {
                if (success && AuthController.isLoggedIn) {
                    authDialog.close()
                } else {
                    authErrorText.text = message
                }
            }
        }
        
        onClosed: {
            authErrorText.text = ""
            passwordInput.text = ""
        }
    }

    // ========================================
    // CLOUD DASHBOARD DIALOG
    // ========================================
    Dialog {
        id: cloudDashboardDialog
        title: "Облачные доски"
        width: 500
        height: 400
        anchors.centerIn: parent
        modal: true
        
        ListView {
            id: boardsListView
            anchors.fill: parent
            model: CloudBoardsManager.cloudBoards
            clip: true
            spacing: 10
            
            delegate: Rectangle {
                width: boardsListView.width
                height: 60
                color: ThemeManager.colors.controlBackground
                radius: 5
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Text {
                        text: modelData.name
                        color: ThemeManager.colors.textColor
                        font.pixelSize: 16
                        Layout.fillWidth: true
                    }
                    
                    Button {
                        text: "Открыть"
                        onClicked: {
                            root.openCloudBoardRequested(modelData.id)
                            cloudDashboardDialog.close()
                        }
                    }
                    
                    Button {
                        text: "Переим."
                        onClicked: {
                            renameBoardDialog.boardId = modelData.id
                            renameBoardDialog.newNameInput.text = modelData.name
                            renameBoardDialog.open()
                        }
                    }
                    
                    Button {
                        text: "Удалить"
                        onClicked: {
                            CloudBoardsManager.deleteBoard(modelData.id)
                        }
                    }
                }
            }
            
            Text {
                anchors.centerIn: parent
                text: "Нет досок"
                color: ThemeManager.colors.textColor
                visible: CloudBoardsManager.cloudBoards.length === 0
            }
        }
    }
    
    // Диалог переименования
    Dialog {
        id: renameBoardDialog
        title: "Переименовать доску"
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        property string boardId: ""
        property alias newNameInput: inputField
        
        ColumnLayout {
            TextField {
                id: inputField
                placeholderText: "Новое имя"
            }
        }
        
        onAccepted: {
            if (inputField.text.trim() !== "") {
                CloudBoardsManager.renameBoard(boardId, inputField.text.trim())
            }
        }
    }

    // Обработка создания доски
    Connections {
        target: CloudBoardsManager
        function onBoardCreated(id, success) {
            if (success) {
                // Если мы находимся в WelcomeWindow и создали доску через New Board
                root.openCloudBoardRequested(id)
            }
        }
    }

    // ========================================
    // UPSCALE MODEL PROMPT
    // ========================================
    MessageDialog {
        id: upscalePromptDialog
        title: "Скачать модель Upscale"
        text: "Для использования функции Upscale (увеличение разрешения с помощью нейросети Real-ESRGAN) необходимо скачать модель (около 35 МБ).\n\nВы хотите скачать её сейчас? Вы всегда сможете сделать это позже в Настройках."
        buttons: MessageDialog.Yes | MessageDialog.No
        onButtonClicked: function(button, role) {
            SettingsManager.hasPromptedUpscale = true
            if (button === MessageDialog.Yes) {
                ModelsManager.downloadModel()
            }
        }
    }
    
    onOpened: {
        if (!SettingsManager.hasPromptedUpscale && !ModelsManager.isModelDownloaded) {
            upscalePromptDialog.open()
        }
    }
}
