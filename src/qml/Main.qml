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
    width: 600 //ширина
    height: 800 //высота
    title: boardController.fileController.windowTitle //заголовок окна берется из контроллера файлов посредством контроллера доски 
    
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
