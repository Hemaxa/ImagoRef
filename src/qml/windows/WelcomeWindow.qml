//WelcomeWindow.qml - стартовое окно приложения

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: root
    
    width: parent.width
    height: parent.height
    anchors.centerIn: parent

    modal: true
    closePolicy: Dialog.NoAutoClose
    padding: 1
    
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
        border.width: 2
        
        topLeftRadius: 0
        topRightRadius: 0
        bottomLeftRadius: 12
        bottomRightRadius: 12

        clip: true
        
        // Декоративные элементы фона
        Image { 
            source: ThemeManager.icons.path_2
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 180; anchors.leftMargin: 120
            scale: 0.9
        }
        Image { 
            source: ThemeManager.icons.rectangles_1
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 163; anchors.rightMargin: -10 
        }
        Image { 
            source: ThemeManager.icons.triangles_1
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 20; anchors.leftMargin: -7
        }
        Image { 
            source: ThemeManager.icons.triangles_2
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 10; anchors.rightMargin: 200 
        }
        Image { 
            source: ThemeManager.icons.star_1
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 340; anchors.leftMargin: 10 
            rotation: 20
            scale: 1.1
        }
        Image { 
            source: ThemeManager.icons.star_1
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 30; anchors.leftMargin: 280
        }
        Image { 
            source: ThemeManager.icons.form_2
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: -50; anchors.leftMargin: 170
            scale: 0.8
        }
        Image { 
            source: ThemeManager.icons.rect_1
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 240; anchors.leftMargin: 5
        }
        Image { 
            source: ThemeManager.icons.path_3
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 290; anchors.rightMargin: 15
        }
        Image { 
            source: ThemeManager.icons.dots
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: -50; anchors.rightMargin: 80
        }
        Image { 
            source: ThemeManager.icons.circles_4
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 170; anchors.leftMargin: -8
        }
        Image { 
            source: ThemeManager.icons.rectangles_2
            anchors.bottom: parent.bottom; anchors.left: parent.left
            anchors.bottomMargin:330; anchors.leftMargin: -15
        }
        Image { 
            source: ThemeManager.icons.star_3
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 180; anchors.rightMargin: 130 
        }
        Image { 
            source: ThemeManager.icons.path_1
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: 145; anchors.rightMargin: 10 
        }
        Image { 
            source: ThemeManager.icons.star_3
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: 300; anchors.rightMargin: 40
        }
        Image { 
            source: ThemeManager.icons.line_2
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 210; anchors.rightMargin: 70 
        }
        Image { 
            source: ThemeManager.icons.line_1
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 20; anchors.leftMargin: 100 
        }
        Image { 
            source: ThemeManager.icons.circles_3
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 190; anchors.leftMargin: 240
            scale: 0.8
        }
        Image { 
            source: ThemeManager.icons.dots
            anchors.bottom: parent.bottom; anchors.left: parent.left
            anchors.bottomMargin: 60; anchors.leftMargin: -50
        }
        Image { 
            source: ThemeManager.icons.form_3
            anchors.bottom: parent.bottom; anchors.left: parent.left
            anchors.bottomMargin: 180; anchors.leftMargin: 5 
        }
        Image { 
            source: ThemeManager.icons.rect_2
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: -8; anchors.rightMargin: 300 
        }
        Image { 
            source: ThemeManager.icons.circles_1
            anchors.bottom: parent.bottom; anchors.left: parent.left
            anchors.bottomMargin: 135; anchors.leftMargin: 220
            scale: 0.9
        }
        Image { 
            source: ThemeManager.icons.form_4
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: 20; anchors.rightMargin: -10
        }
        Image { 
            source: ThemeManager.icons.dots
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: -55; anchors.rightMargin: 50
        }
        Image { 
            source: ThemeManager.icons.circles_2
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: 230; anchors.rightMargin: 10
        }
        Image { 
            source: ThemeManager.icons.lines_star
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: 140; anchors.rightMargin: 180 
        }
        Image { 
            source: ThemeManager.icons.form_1
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 240; anchors.rightMargin: -5
            scale: 0.9
        }
        Image { 
            source: ThemeManager.icons.line_3
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 220; anchors.rightMargin: 100
        }
        Image { 
            source: ThemeManager.icons.form_4
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 180; anchors.rightMargin: 200
            scale: 0.8
            rotation: 30
        }
        Image { 
            source: ThemeManager.icons.line_1
            anchors.bottom: parent.bottom; anchors.right: parent.right
            anchors.bottomMargin: 20; anchors.rightMargin: 220
        }
        Image { 
            source: ThemeManager.icons.star_3
            anchors.bottom: parent.bottom; anchors.left: parent.left
            anchors.bottomMargin: 260; anchors.leftMargin: 10 
        }
        Image { 
            source: ThemeManager.icons.lines_star
            anchors.top: parent.top; anchors.left: parent.left
            anchors.topMargin: 170; anchors.leftMargin: 70
            scale: 0.9
            rotation: 70
        }
        Image { 
            source: ThemeManager.icons.lines_star
            anchors.top: parent.top; anchors.right: parent.right
            anchors.topMargin: 240; anchors.rightMargin: 150
            scale: 1.1
            rotation: -30
        }
    }
    
    // ========================================
    // MAIN CONTENT
    // ========================================
    Item {
        anchors.fill: parent
        
        //Логотип
        Image {
            id: logoImage
            source: ThemeManager.icons.logo
            anchors.horizontalCenter: parent.horizontalCenter
            y: 35
            width: 560
            height: 180
            fillMode: Image.PreserveAspectFit
        }
        
        //Заголовок "Recent projects"
        Image {
            id: sectionHeader
            source: ThemeManager.icons.recent_projects
            anchors.top: logoImage.bottom
            anchors.topMargin: 30
            anchors.left: parent.left
            anchors.leftMargin: 50
            width: 360
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
                id: authBtnRect
                anchors.fill: parent
                radius: 20
                clip: true
                color: AuthController.isLoggedIn ? ThemeManager.colors.controlBackground : root.accentYellow
                border.color: ThemeManager.colors.controlBorder
                border.width: 1

                property string initialStr: {
                    if (!AuthController.isLoggedIn) return "Войти"
                    if (AuthController.userNickname !== "") return AuthController.userNickname.charAt(0).toUpperCase()
                    if (AuthController.userEmail !== "") return AuthController.userEmail.charAt(0).toUpperCase()
                    return "?"
                }

                Text {
                    id: authBtnText
                    anchors.centerIn: parent
                    text: authBtnRect.initialStr
                    font.pixelSize: AuthController.isLoggedIn ? 18 : 14
                    font.bold: true
                    color: AuthController.isLoggedIn ? ThemeManager.colors.textColor : root.textDark
                    visible: !AuthController.isLoggedIn || AuthController.userAvatarHash === ""
                }
                
                Image {
                    id: userAvatarImage
                    anchors.fill: parent
                    anchors.margins: 1
                    source: AuthController.isLoggedIn && AuthController.userAvatarHash !== "" ? "https://imagoref.s3.timeweb.com/" + AuthController.userAvatarHash : ""
                    fillMode: Image.PreserveAspectCrop
                    visible: AuthController.isLoggedIn && AuthController.userAvatarHash !== ""
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
                        text: "Профиль"
                        onTriggered: {
                            profileDialog.nicknameInput.text = AuthController.userNickname
                            profileDialog.avatarPath = ""
                            profileDialog.open()
                        }
                    }
                    MenuItem {
                        text: "Доски"
                        onTriggered: {
                            BoardsManager.loadBoards()
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
            rowSpacing: 25
            columnSpacing: 25
            
            Repeater {
                model: 6
                
                // Карточка проекта
                Item {
                    width: 140
                    height: 140
                    
                    property var projData: index < SettingsManager.recentBoards.length ? SettingsManager.recentBoards[index] : null
                    
                    // Рамка
                    Image {
                        id: frameImage
                        anchors.fill: parent
                        source: ThemeManager.icons.frame || ""
                        // Используем PreserveAspectFit вместо Stretch, чтобы рамка не искажалась
                        fillMode: Image.PreserveAspectFit 
                        visible: true
                    }
                    
                    // Контейнер для скриншота (предпросмотра)
                    Rectangle {
                        anchors.fill: parent
                        // Оставьте нужный отступ, чтобы попасть точно в прозрачное/серое "окно" вашей SVG-рамки
                        anchors.margins: 12 
                        
                        // Делаем фон прозрачным, чтобы серый фон из SVG-рамки был виден
                        color: "transparent" 
                        radius: 5
                        clip: true
                        
                        Image {
                            anchors.fill: parent
                            source: projData ? (projData.previewPath ? projData.previewPath : "") : ""
                            // Сохраняем PreserveAspectCrop, чтобы скриншот заполнял область без сплющивания
                            fillMode: Image.PreserveAspectCrop 
                            visible: projData !== null && projData.previewPath && projData.previewPath !== ""
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: projData !== null ? Qt.PointingHandCursor : Qt.ArrowCursor
                        hoverEnabled: projData !== null
                        
                        onEntered: {
                            if (projData !== null) parent.scale = 1.05
                        }
                        onExited: {
                            parent.scale = 1.0
                        }

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
                    
                    Behavior on scale {
                        NumberAnimation { duration: 100 }
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
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            width: 230
            fillMode: Image.PreserveAspectFit
            verticalAlignment: Image.AlignBottom
        }
        
        // ========================================
        // BUTTONS (кнопки рядом с персонажем)
        // ========================================
        Row {
            id: buttonsRow
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 60
            spacing: 10
            
            Image {
                id: newBoardBtn
                source: ThemeManager.icons.button_1 || ""
                width: 156
                fillMode: Image.PreserveAspectFit

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        if (AuthController.isLoggedIn) {
                            BoardsManager.createBoard("New Board")
                        } else {
                            root.newBoardRequested()
                        }
                    }

                    onEntered: newBoardBtn.scale = 1.05
                    onExited: newBoardBtn.scale = 1.0
                }

                Behavior on scale { 
                    NumberAnimation { duration: 100 } 
                }
            }

            // Кнопка "Open Existing"
            Image {
                id: openExistingBtn
                source: ThemeManager.icons.button_2 || ""
                width: 156
                fillMode: Image.PreserveAspectFit

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
                                BoardsManager.loadBoards()
                                cloudDashboardDialog.open()
                            }
                        }
                    }

                    onEntered: openExistingBtn.scale = 1.05
                    onExited: openExistingBtn.scale = 1.0
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
    // PROFILE DIALOG
    // ========================================
    Dialog {
        id: profileDialog
        title: "Профиль"
        width: 350
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Save | Dialog.Cancel
        
        property string avatarPath: ""
        property alias nicknameInput: nicknameField
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 15
            
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                width: 80
                height: 80
                radius: 40
                color: ThemeManager.colors.controlBackground
                border.color: ThemeManager.colors.controlBorder
                border.width: 1
                clip: true
                
                Image {
                    id: profileAvatarImage
                    anchors.fill: parent
                    anchors.margins: 1
                    fillMode: Image.PreserveAspectCrop
                    source: {
                        if (profileDialog.avatarPath !== "") return profileDialog.avatarPath;
                        if (AuthController.userAvatarHash !== "") return "https://imagoref.s3.timeweb.com/" + AuthController.userAvatarHash;
                        return "";
                    }
                    visible: source !== ""
                }
                
                Text {
                    anchors.centerIn: parent
                    text: AuthController.userNickname !== "" ? AuthController.userNickname.charAt(0).toUpperCase() : (AuthController.userEmail !== "" ? AuthController.userEmail.charAt(0).toUpperCase() : "?")
                    font.pixelSize: 24
                    color: ThemeManager.colors.textColor
                    visible: profileAvatarImage.source == ""
                }
                
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: avatarFileDialog.open()
                }
            }
            
            Button {
                Layout.alignment: Qt.AlignHCenter
                text: "Выбрать аватарку"
                onClicked: avatarFileDialog.open()
            }
            
            TextField {
                id: nicknameField
                Layout.fillWidth: true
                placeholderText: "Никнейм"
            }
        }
        
        onAccepted: {
            AuthController.updateProfile(nicknameField.text, avatarPath)
        }
    }
    
    FileDialog {
        id: avatarFileDialog
        title: "Выбрать аватарку"
        nameFilters: ["Изображения (*.png *.jpg *.jpeg)"]
        onAccepted: {
            profileDialog.avatarPath = selectedFile
        }
    }

    // ========================================
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
            model: BoardsManager.boards
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
                            BoardsManager.deleteBoard(modelData.id)
                        }
                    }
                }
            }
            
            Text {
                anchors.centerIn: parent
                text: "Нет досок"
                color: ThemeManager.colors.textColor
                visible: BoardsManager.boards.length === 0
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
                BoardsManager.renameBoard(boardId, inputField.text.trim())
            }
        }
    }

    // Обработка создания доски
    Connections {
        target: BoardsManager
        function onBoardCreated(boardId, success) {
            if (success) {
                // Если мы находимся в WelcomeWindow и создали доску через New Board
                root.openCloudBoardRequested(boardId)
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
