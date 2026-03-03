import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Dialogs
import ImagoRef
import "windows"
import "components"

// Main.qml - главная точка входа QML приложения. Управляет показом WelcomeWindow, MainWindow и SettingsWindow.
ApplicationWindow {
    id: root
    visible: true
    width: 900
    height: 700
    title: boardController.windowTitle
    color: Theme.backgroundColor
    
    // Контроллер доски
    BoardController {
        id: boardController
    }
    
    // Системная панель меню (нативная macOS)
    menuBar: MenuBar {
        Menu {
            title: "Файл"
            
            Action {
                text: "Открыть..."
                shortcut: StandardKey.Open
                onTriggered: {
                    if (mainLoader.active) {
                        fileOpenDialog.open()
                    }
                }
            }
            MenuSeparator {}
            Action {
                text: "Сохранить"
                shortcut: StandardKey.Save
                onTriggered: {
                    if (mainLoader.active) {
                        if (boardController.currentFilePath !== "") {
                            boardController.saveBoard()
                        } else {
                            fileSaveDialog.open()
                        }
                    }
                }
            }
            Action {
                text: "Сохранить как..."
                shortcut: StandardKey.SaveAs
                onTriggered: {
                    if (mainLoader.active) {
                        fileSaveDialog.open()
                    }
                }
            }
        }
    }
    
    // Загрузчик главного окна
    Loader {
        id: mainLoader
        active: false
        anchors.fill: parent
        sourceComponent: MainWindow {
            controller: boardController
            onSettingsRequested: settingsDialog.open()
        }
    }
    
    // Диалог настроек
    SettingsWindow {
        id: settingsDialog
        anchors.centerIn: parent
    }
    
    // Диалог приветствия
    WelcomeWindow {
        id: welcomeDialog
        anchors.centerIn: parent
        
        onNewBoardRequested: {
            root.showMaximized()
            boardController.newBoard()
            mainLoader.active = true
            welcomeDialog.close()
        }
        
        onOpenBoardRequested: function(fileUrl) {
            if (boardController.openBoard(fileUrl)) {
                root.showMaximized()
                mainLoader.active = true
                welcomeDialog.close()
            }
        }
    }
    
    // Диалог сохранения
    FileDialog {
        id: fileSaveDialog
        title: "Сохранить доску"
        fileMode: FileDialog.SaveFile
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: boardController.saveBoardAs(selectedFile)
    }
    
    // Диалог открытия
    FileDialog {
        id: fileOpenDialog
        title: "Открыть доску"
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: boardController.openBoard(selectedFile)
    }
    
    // Показать окно приветствия при запуске
    Component.onCompleted: {
        welcomeDialog.open()
    }
    
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: settingsDialog.open()
    }
}
