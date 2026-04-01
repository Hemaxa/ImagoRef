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
        border.color: "#181818"
        border.width: 4
        radius: 8
        
        // Декоративные элементы фона
        Image { 
            source: ThemeManager.icons.dots
            anchors.right: parent.right
            anchors.rightMargin: 40
            anchors.top: parent.top
            anchors.topMargin: 20
        }
        Image { 
            source: ThemeManager.icons.dots
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
        }
        Image { 
            source: ThemeManager.icons.dots
            anchors.right: parent.right
            anchors.rightMargin: 40
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
        }
        Image { 
            source: ThemeManager.icons.rectangles_1
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 50
        }
        Image { 
            source: ThemeManager.icons.path_1
            anchors.left: parent.left
            anchors.leftMargin: 100
            anchors.top: parent.top
            anchors.topMargin: 20
        }
        Image { 
            source: ThemeManager.icons.triangles_1
            anchors.left: parent.left
            anchors.leftMargin: 190
            anchors.top: parent.top
            anchors.topMargin: 20
        }
        Image { 
            source: ThemeManager.icons.circles_1
            anchors.right: parent.right
            anchors.rightMargin: 160
            anchors.top: parent.top
            anchors.topMargin: 15
        }

        Image { 
            source: ThemeManager.icons.star_1
            anchors.left: parent.left
            anchors.leftMargin: 70
            anchors.top: parent.top
            anchors.topMargin: 180
        }
        Image { 
            source: ThemeManager.icons.line_1
            anchors.left: parent.left
            anchors.leftMargin: 160
            anchors.top: parent.top
            anchors.topMargin: 200
        }
        Image { 
            source: ThemeManager.icons.circles_2
            anchors.left: parent.left
            anchors.leftMargin: 280
            anchors.top: parent.top
            anchors.topMargin: 220
        }
        Image { 
            source: ThemeManager.icons.star_2
            anchors.right: parent.right
            anchors.rightMargin: 130
            anchors.top: parent.top
            anchors.topMargin: 210
        }
        Image { 
            source: ThemeManager.icons.path_2
            anchors.right: parent.right
            anchors.rightMargin: 60
            anchors.top: parent.top
            anchors.topMargin: 230
        }

        Image { 
            source: ThemeManager.icons.rect_1
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 250
        }
        Image { 
            source: ThemeManager.icons.star_3
            anchors.right: parent.right
            anchors.rightMargin: 30
            anchors.top: parent.top
            anchors.topMargin: 270
        }

        Image { 
            source: ThemeManager.icons.path_3
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.top: parent.top
            anchors.topMargin: 310
        }
        Image { 
            source: ThemeManager.icons.line_2
            anchors.right: parent.right
            anchors.rightMargin: 90
            anchors.top: parent.top
            anchors.topMargin: 310
        }

        Image { 
            source: ThemeManager.icons.form_1
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 280
        }
        Image { 
            source: ThemeManager.icons.circles_3
            anchors.right: parent.right
            anchors.rightMargin: 30
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 250
        }

        Image { 
            source: ThemeManager.icons.rect_2
            anchors.left: parent.left
            anchors.leftMargin: 25
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 150
        }
        Image { 
            source: ThemeManager.icons.circles_4
            anchors.left: parent.left
            anchors.leftMargin: 220
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 140
        }

        Image { 
            source: ThemeManager.icons.lines_star
            anchors.right: parent.right
            anchors.rightMargin: 180
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 140
        }
        Image { 
            source: ThemeManager.icons.form_2
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 130
        }
        Image { 
            source: ThemeManager.icons.line_3
            anchors.right: parent.right
            anchors.rightMargin: 50
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 170
        }
        Image { 
            source: ThemeManager.icons.form_3
            anchors.right: parent.right
            anchors.rightMargin: 200
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
        }
        Image { 
            source: ThemeManager.icons.form_4
            anchors.left: parent.left
            anchors.leftMargin: 280
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
        }
        Image { 
            source: ThemeManager.icons.triangles_2
            anchors.left: parent.left
            anchors.leftMargin: 90
            anchors.top: parent.top
            anchors.topMargin: 150
        }
        Image { 
            source: ThemeManager.icons.rectangles_2
            anchors.right: parent.right
            anchors.rightMargin: 120
            anchors.top: parent.top
            anchors.topMargin: 120
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
            y: 70
            width: 520
            height: 140
            fillMode: Image.PreserveAspectFit
        }
        
        // Заголовок "Recent projects"
        Image {
            id: sectionHeader
            source: ThemeManager.icons.recent_projects
            anchors.top: logoImage.bottom
            anchors.topMargin: 30
            anchors.left: parent.left
            anchors.leftMargin: 40
            width: 380
            height: 50
            fillMode: Image.PreserveAspectFit
        }

        // Кнопка авторизации / Аватар пользователя (В правом верхнем углу)
        Item {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 20
            anchors.rightMargin: 20
            width: AuthController.isLoggedIn ? 40 : 80
            height: 40

            Rectangle {
                anchors.fill: parent
                radius: 20
                color: AuthController.isLoggedIn ? ThemeManager.colors.controlBackground : root.accentYellow
                border.color: ThemeManager.colors.controlBorder
                border.width: 1

                Text {
                    id: authBtnText
                    anchors.centerIn: parent
                    text: AuthController.isLoggedIn ? AuthController.userEmail.charAt(0).toUpperCase() : "Войти"
                    font.pixelSize: AuthController.isLoggedIn ? 18 : 14
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
            anchors.topMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            
            columns: 3
            rowSpacing: 15
            columnSpacing: 25
            
            Repeater {
                model: 6
                
                // Карточка проекта
                Item {
                    width: 160
                    height: 160
                    
                    property var projData: index < SettingsManager.recentBoards.length ? SettingsManager.recentBoards[index] : null
                    
                    // Рамка с полосками
                    Image {
                        id: frameImage
                        anchors.fill: parent
                        source: ThemeManager.icons.frame || ""
                        fillMode: Image.Stretch
                        visible: true
                    }
                    
                    // Превью проекта или "пустой" квадрат
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 12 // Уменьшил отступ, чтобы больше покрывать белый фон внутри SVG
                        color: root.bgColor // ОРАНЖЕВЫЙ, чтобы перекрыть белый пустой фон рамки
                        radius: 5
                        clip: true
                        
                        Image {
                            anchors.fill: parent
                            source: projData ? (projData.previewPath ? projData.previewPath : "") : ""
                            fillMode: Image.PreserveAspectCrop
                            visible: projData !== null && projData.previewPath && projData.previewPath !== ""
                        }
                        
                        // Иконка типа
                        // Удалили отсутствующие иконки для чистой консоли
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: projData !== null ? Qt.PointingHandCursor : Qt.ArrowCursor
                        onClicked: {
                            if (projData !== null) {
                                if (projData.type === "cloud") {
                                    root.openCloudBoardRequested(projData.id)
                                } else {
                                    root.openBoardRequested(Qt.url(projData.path))
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // ========================================
        // CHARACTER (персонаж слева внизу)
        // ========================================
        Image {
            id: mascotImage
            source: ThemeManager.icons.character || ""
            anchors.left: parent.left
            anchors.leftMargin: 40
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            width: 170
            height: 230
            fillMode: Image.PreserveAspectFit
            verticalAlignment: Image.AlignBottom
        }
        
        // ========================================
        // BUTTONS (кнопки рядом с персонажем)
        // ========================================
        Row {
            id: buttonsRow
            anchors.left: mascotImage.right
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            spacing: 20
            
            // Кнопка "New Board"
            Item {
                id: newBoardBtn
                width: 160
                height: 60
                
                Image {
                    anchors.fill: parent
                    source: ThemeManager.icons.button_1 || ""
                    fillMode: Image.Stretch
                }
                
                // Удалили QML Text, так как он зашит внутрь button_1.svg
                
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
            Item {
                id: openExistingBtn
                width: 160
                height: 60
                
                Image {
                    anchors.fill: parent
                    source: ThemeManager.icons.button_2 || ""
                    fillMode: Image.Stretch
                }
                
                // Удалили QML Text, так как он зашит внутрь button_2.svg!
                
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
