import QtQuick
import QtQuick.Controls
import QtQuick.Window
import ImagoRef

/**
 * Main.qml - главная точка входа QML приложения.
 * Управляет показом WelcomeWindow и MainWindow.
 */
ApplicationWindow {
    id: root
    
    visible: true
    width: 1280
    height: 800
    title: boardController.windowTitle
    color: Theme.backgroundColor
    
    // Контроллер доски
    BoardController {
        id: boardController
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
            boardController.newBoard()
            mainLoader.active = true
            welcomeDialog.close()
        }
        
        onOpenBoardRequested: function(fileUrl) {
            if (boardController.openBoard(fileUrl)) {
                mainLoader.active = true
                welcomeDialog.close()
            }
        }
    }
    
    // Показать окно приветствия при запуске
    Component.onCompleted: {
        welcomeDialog.open()
    }
    
    // Глобальные горячие клавиши
    Shortcut {
        sequence: StandardKey.Open
        onActivated: {
            if (mainLoader.active) {
                fileOpenDialog.open()
            }
        }
    }
    
    Shortcut {
        sequence: StandardKey.Save
        onActivated: {
            if (mainLoader.active && boardController.currentFilePath !== "") {
                boardController.saveBoard()
            } else if (mainLoader.active) {
                fileSaveDialog.open()
            }
        }
    }
    
    Shortcut {
        sequence: StandardKey.SaveAs
        onActivated: {
            if (mainLoader.active) {
                fileSaveDialog.open()
            }
        }
    }
    
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: settingsDialog.open()
    }
}
