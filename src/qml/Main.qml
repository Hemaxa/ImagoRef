//Main.qml - точка входа для QML части приложения, управляет показом WelcomeWindow, MainWindow и SettingsWindow

import QtQuick
import QtQuick.Controls //добавляет базовые элементы UI
import QtQuick.Window //работа с окнами и экраном
import QtQuick.Dialogs //работа с нативными диалогами операционной системы
import ImagoRef //добавление виртуального модуля приложения

import "windows" //подключает все .qml файлы из папки windows
import "components" //подключает все .qml файлы из папки components

//корневое окно приложения
ApplicationWindow {
    id: root //идентификатор (root является корневым элементом только для этого файла)
    visible: true //режим видимости
    width: 900 //ширина
    height: 700 //высота
    title: boardController.fileController.windowTitle //заголовок окна берется из контроллера файлов посредством контроллера доски 
    color: ThemeManager.backgroundColor //цвет фона берется из менеджера тем
    
    //создает экземпляр контроллера доски
    BoardController {
        id: boardController
    }
    
    //создает системную панель меню
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

            MenuSeparator {} //разделитель

            Action {
                text: "Сохранить"
                shortcut: StandardKey.Save
                onTriggered: {
                    if (mainLoader.active) {
                        if (boardController.fileController.currentFilePath !== "") {
                            boardController.fileController.saveBoard()
                        }
                        else {
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
            root.showMaximized()
            boardController.fileController.newBoard()
            mainLoader.active = true
            welcomeDialog.close()
        }
        
        //открытие существующей доски
        onOpenBoardRequested: function(fileUrl) {
            if (boardController.fileController.openBoard(fileUrl)) {
                root.showMaximized()
                mainLoader.active = true
                welcomeDialog.close()
            }
        }
    }
    
    //создание системных диалогов
    //диалог сохранения файла
    FileDialog {
        id: fileSaveDialog
        title: "Сохранить доску"
        fileMode: FileDialog.SaveFile
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: boardController.fileController.saveBoardAs(selectedFile)
    }
    
    //диалог открытия файла
    FileDialog {
        id: fileOpenDialog
        title: "Открыть доску"
        nameFilters: ["ImagoRef доска (*.iref)", "Все файлы (*)"]
        onAccepted: boardController.fileController.openBoard(selectedFile)
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
