//Main.qml - точка входа для QML части приложения, управляет показом WelcomeWindow, MainWindow и SettingsWindow

import QtQuick
import QtQuick.Controls //добавляет базовые элементы UI
import QtQuick.Window //работа с окнами и экраном
import QtQuick.Dialogs //работа с нативными диалогами операционной системы
import QtQuick.Layouts //работа с лэйаутами
import ImagoRef //добавление виртуального модуля приложения

import "windows" //подключает все .qml файлы из папки windows
import "components" //подключает все .qml файлы из папки components

//корневое окно приложения
ApplicationWindow {
    id: root //идентификатор (root является корневым элементом только для этого файла)
    visible: true //режим видимости
    width: 600 //ширина
    height: 800 //высота
    title: boardController.storageController.windowTitle //заголовок окна берется из контроллера файлов посредством контроллера доски 
    
    // Свойства для режима закрепления
    property bool isPinned: boardController.toolController.isPinned
    property bool isPinnedAndInactive: isPinned && !Qt.application.active
    
    color: "transparent" // Задаем всегда прозрачный цвет окна, чтобы у macOS был включен альфа-канал

    background: Rectangle {
        color: root.isPinnedAndInactive ? "transparent" : ThemeManager.colors.backgroundColor
    }

    flags: {
        var baseFlags = Qt.Window
        if (isPinned) {
            baseFlags |= Qt.WindowStaysOnTopHint
        }
        if (isPinnedAndInactive) {
            baseFlags |= Qt.FramelessWindowHint | Qt.WindowTransparentForInput
        }
        return baseFlags
    }

    
    //создает экземпляр контроллера доски
    BoardController {
        id: boardController
    }
    
    //создает системную панель меню
    menuBar: MenuBar {
        Menu {
            title: "Файл"
            
            Action {
                text: "На стартовый экран"
                onTriggered: {
                    mainLoader.active = false
                    // Возвращаем окно в исходное состояние
                    root.showNormal()
                    root.width = 600
                    root.height = 800
                    welcomeDialog.open()
                }
            }

            MenuSeparator {}

            Action {
                text: "Открыть..."
                shortcut: StandardKey.Open
                onTriggered: {
                    if (mainLoader.active) {
                        if (AuthController.isLoggedIn) {
                            BoardsManager.loadBoards()
                            mainCloudDashboard.open()
                        } else {
                            fileOpenDialog.open()
                        }
                    }
                }
            }
            Action {
                text: "Открыть из файла"
                onTriggered: {
                    if (mainLoader.active) {
                        fileOpenDialog.open()
                    }
                }
            }

            MenuSeparator {} //разделитель

            Action {
                text: "Сохранить"
                shortcut: StandardKey.Save
                onTriggered: {
                    if (mainLoader.active) {
                        if (AuthController.isLoggedIn) {
                            // Облачная доска уже привязана — синхронизируем
                            if (boardController.currentBoardId !== "") {
                                boardController.networkController.syncBoardToServer()
                            }
                            // Новая доска без ID — предлагаем сохранить в облако
                            else {
                                var defaultName = boardController.storageController.windowTitle.replace("ImagoRef - ", "")
                                if (defaultName === "" || defaultName === "ImagoRef") {
                                    defaultName = "New Board"
                                }
                                newCloudBoardNameInput.text = defaultName
                                newCloudBoardDialog.open()
                            }
                        } else {
                            // Не авторизован — сохраняем локально
                            if (boardController.storageController.currentFilePath !== "") {
                                boardController.storageController.saveBoard()
                            } else {
                                fileSaveDialog.open()
                            }
                        }
                    }
                }
            }
            Action {
                text: "Сохранить как..."
                shortcut: StandardKey.SaveAs
                onTriggered: {
                    if (mainLoader.active) {
                        if (AuthController.isLoggedIn) {
                            var copyName = boardController.storageController.windowTitle.replace("ImagoRef - ", "")
                            if (copyName === "" || copyName === "ImagoRef") {
                                copyName = "New Board"
                            }
                            newCloudBoardNameInput.text = copyName + " (Копия)"
                            newCloudBoardDialog.open()
                        } else {
                            fileSaveDialog.open()
                        }
                    }
                }
            }
            Action {
                text: "Сохранить как файл"
                onTriggered: {
                    if (mainLoader.active) {
                        fileSaveDialog.open()
                    }
                }
            }
        }
        
        Menu {
            title: "Инструменты"
            
            Action {
                id: actionZoomIn
                text: "Приблизить"
                checkable: true
                checked: SettingsManager.isToolEnabled("ZoomIn")
                onCheckedChanged: SettingsManager.setToolEnabled("ZoomIn", actionZoomIn.checked)
            }
            Action {
                id: actionZoomOut
                text: "Отдалить"
                checkable: true
                checked: SettingsManager.isToolEnabled("ZoomOut")
                onCheckedChanged: SettingsManager.setToolEnabled("ZoomOut", actionZoomOut.checked)
            }
            Action {
                id: actionResize
                text: "Изменить размер"
                checkable: true
                checked: SettingsManager.isToolEnabled("Resize")
                onCheckedChanged: SettingsManager.setToolEnabled("Resize", actionResize.checked)
            }
            Action {
                id: actionCrop
                text: "Обрезать"
                checkable: true
                checked: SettingsManager.isToolEnabled("Crop")
                onCheckedChanged: SettingsManager.setToolEnabled("Crop", actionCrop.checked)
            }
            Action {
                id: actionLabel
                text: "Надпись"
                checkable: true
                checked: SettingsManager.isToolEnabled("Label")
                onCheckedChanged: SettingsManager.setToolEnabled("Label", actionLabel.checked)
            }
            Action {
                id: actionOpacity
                text: "Непрозрачность"
                checkable: true
                checked: SettingsManager.isToolEnabled("Opacity")
                onCheckedChanged: SettingsManager.setToolEnabled("Opacity", actionOpacity.checked)
            }
            Action {
                id: actionArrange
                text: "Расположить"
                checkable: true
                checked: SettingsManager.isToolEnabled("Arrange")
                onCheckedChanged: SettingsManager.setToolEnabled("Arrange", actionArrange.checked)
            }
            Action {
                id: actionEyedropper
                text: "Пипетка"
                checkable: true
                checked: SettingsManager.isToolEnabled("Eyedropper")
                onCheckedChanged: SettingsManager.setToolEnabled("Eyedropper", actionEyedropper.checked)
            }
            
            MenuSeparator {}
            
            Action {
                id: actionPaste
                text: "Вставить из буфера"
                checkable: true
                checked: SettingsManager.isToolEnabled("Paste")
                onCheckedChanged: SettingsManager.setToolEnabled("Paste", actionPaste.checked)
            }
            Action {
                id: actionDelete
                text: "Удалить"
                checkable: true
                checked: SettingsManager.isToolEnabled("Delete")
                onCheckedChanged: SettingsManager.setToolEnabled("Delete", actionDelete.checked)
            }
            Action {
                id: actionGridSnap
                text: "Привязать к сетке"
                checkable: true
                checked: SettingsManager.isToolEnabled("GridSnap")
                onCheckedChanged: SettingsManager.setToolEnabled("GridSnap", actionGridSnap.checked)
            }
            Action {
                id: actionUpscale
                text: "Увеличить разрешение"
                checkable: true
                checked: SettingsManager.isToolEnabled("Upscale")
                onCheckedChanged: SettingsManager.setToolEnabled("Upscale", actionUpscale.checked)
            }
            Action {
                id: actionRotate
                text: "Повернуть"
                checkable: true
                checked: SettingsManager.isToolEnabled("Rotate")
                onCheckedChanged: SettingsManager.setToolEnabled("Rotate", actionRotate.checked)
            }
            Action {
                id: actionUndoRedo
                text: "Отменить/Повторить"
                checkable: true
                checked: SettingsManager.isToolEnabled("UndoRedo")
                onCheckedChanged: SettingsManager.setToolEnabled("UndoRedo", actionUndoRedo.checked)
            }
            Action {
                id: actionPin
                text: "Закрепить окно"
                checkable: true
                checked: SettingsManager.isToolEnabled("Pin")
                onCheckedChanged: SettingsManager.setToolEnabled("Pin", actionPin.checked)
            }
            Action {
                id: actionSettingsBtn
                text: "Кнопка настроек"
                checkable: true
                checked: SettingsManager.isToolEnabled("SettingsBtn")
                onCheckedChanged: SettingsManager.setToolEnabled("SettingsBtn", actionSettingsBtn.checked)
            }
        }
    }
    
    //загрузчик главного окна
    Loader {
        id: mainLoader
        active: false
        anchors.fill: parent
        sourceComponent: MainWindow {
            controller: boardController
            onSettingsRequested: settingsDialog.open()
        }
    }
    
    //создает экземпляр диалога настроек
    SettingsWindow {
        id: settingsDialog
        anchors.centerIn: parent
    }
    
    //создает экземпляр диалога приветствия
    WelcomeWindow {
        id: welcomeDialog
        anchors.centerIn: parent
        
        //создание новой доски
        onNewBoardRequested: {
            boardController.currentBoardId = ""
            root.showMaximized()
            boardController.storageController.newBoard()
            mainLoader.active = true
            welcomeDialog.close()
        }
        
        //открытие существующей доски
        onOpenBoardRequested: function(fileUrl) {
            boardController.currentBoardId = ""
            if (boardController.storageController.openBoard(fileUrl)) {
                root.addLocalToHistory(fileUrl)
                root.showMaximized()
                mainLoader.active = true
                welcomeDialog.close()
            }
        }

        //открытие облачной доски
        onOpenCloudBoardRequested: function(boardId) {
            boardController.currentBoardId = boardId
            root.addCloudToHistory(boardId)
            root.showMaximized()
            mainLoader.active = true
            welcomeDialog.close()

            boardController.networkController.connectToBoard(boardId)
        }
    }
    
    //создание системных диалогов
    //диалог сохранения файла
    FileDialog {
        id: fileSaveDialog
        title: "Сохранить доску"
        fileMode: FileDialog.SaveFile
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: {
            boardController.storageController.saveBoardAs(selectedFile)
            root.addLocalToHistory(selectedFile)
        }
    }
    
    //диалог открытия файла
    FileDialog {
        id: fileOpenDialog
        title: "Открыть доску"
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: {
            if (boardController.storageController.openBoard(selectedFile)) {
                root.addLocalToHistory(selectedFile)
            }
        }
    }
    
    // Диалог списка облачных досок (из меню)
    Dialog {
        id: mainCloudDashboard
        title: "Облачные доски"
        width: 500
        height: 400
        anchors.centerIn: parent
        modal: true

        background: Rectangle {
            color: ThemeManager.colors.backgroundColor
            border.color: ThemeManager.colors.accentColor
            border.width: 1
            radius: 8
        }
        
        ListView {
            anchors.fill: parent
            model: BoardsManager.boards
            clip: true
            spacing: 10
            
            delegate: Rectangle {
                width: parent ? parent.width : 0
                height: 50
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
                            root.addCloudToHistory(modelData.id)
                            root.showMaximized()
                            mainLoader.active = true
                            mainCloudDashboard.close()
                            boardController.openCloudBoard(modelData.id)
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

    // Диалог для ввода имени новой облачной доски
    Dialog {
        id: newCloudBoardDialog
        title: "Сохранить в облако"
        anchors.centerIn: parent
        modal: true
        width: 380
        padding: 20

        background: Rectangle {
            color: ThemeManager.colors.backgroundColor
            border.color: ThemeManager.colors.controlBackground
            border.width: 1
            radius: 8
        }

        header: Item {
            height: 40
            Text {
                anchors.centerIn: parent
                text: "Сохранить в облако"
                color: ThemeManager.colors.textColor
                font.pixelSize: 16
                font.bold: true
            }
        }

        contentItem: ColumnLayout {
            spacing: 16

            Text {
                text: "Имя доски:"
                color: ThemeManager.colors.textColor
                font.pixelSize: 13
            }

            TextField {
                id: newCloudBoardNameInput
                Layout.fillWidth: true
                placeholderText: "Введите имя доски"
                text: "New Board"
                color: ThemeManager.colors.textColor
                font.pixelSize: 14
                selectByMouse: true

                background: Rectangle {
                    color: ThemeManager.colors.controlBackground
                    border.color: newCloudBoardNameInput.activeFocus ? ThemeManager.colors.accentColor : ThemeManager.colors.controlBackground
                    border.width: 1
                    radius: 4
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                spacing: 10

                Item { Layout.fillWidth: true } // спейсер

                Button {
                    text: "Отмена"
                    onClicked: newCloudBoardDialog.close()
                    contentItem: Text {
                        text: parent.text
                        color: ThemeManager.colors.textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 90
                        implicitHeight: 32
                        color: parent.hovered ? ThemeManager.colors.controlBackground : "transparent"
                        border.color: ThemeManager.colors.controlBackground
                        border.width: 1
                        radius: 4
                    }
                }

                Button {
                    text: "Сохранить"
                    onClicked: {
                        newCloudBoardDialog.accepted()
                        newCloudBoardDialog.close()
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 32
                        color: parent.hovered ? Qt.darker(ThemeManager.colors.accentColor, 1.15) : ThemeManager.colors.accentColor
                        radius: 4
                    }
                }
            }
        }

        onAccepted: {
            if (newCloudBoardNameInput.text.trim() !== "") {
                BoardsManager.createBoard(newCloudBoardNameInput.text.trim())
            }
        }
    }

    // Обработка создания доски из меню "Сохранить в облако"
    Connections {
        target: BoardsManager
        function onBoardCreated(id, success) {
            if (success && mainLoader.active && boardController.currentBoardId === "") {
                boardController.currentBoardId = id
                root.addCloudToHistory(id)

                boardController.networkController.connectToBoard(id)
            }
        }
    }

    function addLocalToHistory(fileUrl) {
        var str = fileUrl.toString()
        var name = str
        var idx = str.lastIndexOf("/")
        if (idx >= 0) name = str.substring(idx + 1)
        name = name.replace(".iref", "") // убираем расширение
        
        SettingsManager.addRecentBoard({
            "id": "",
            "path": str,
            "name": name,
            "type": "local"
        })
    }

    function addCloudToHistory(boardId) {
        var name = "Cloud Board"
        for (var i = 0; i < BoardsManager.boards.length; i++) {
            if (BoardsManager.boards[i].id === boardId) {
                name = BoardsManager.boards[i].name
                break
            }
        }
        SettingsManager.addRecentBoard({
            "id": boardId,
            "path": "",
            "name": name,
            "type": "cloud"
        })
    }

    //показать окно приветствия при запуске после исполнения всего файла
    Component.onCompleted: {
        welcomeDialog.open()
    }
    
    //горячие клавиши
    //открытие окна настроек
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: settingsDialog.open()
    }
}
