//main.cpp - точка входа для всего приложения, инициализирует QML движок, регистрирует C++ типы, запускает бесконечный цикл событий

#include <QGuiApplication> //основной класс для GUI-приложений без виджетов
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>//движок, который загружает и управляет QML
#include <QQuickStyle> //стили для QML
#include <QIcon> //иконки для QML
#include <QSurfaceFormat> //формат поверхности

#include "SettingsManager.h"
#include "ThemesManager.h"
#include "ImageProvider.h"
#include "ModelsManager.h"
#include "AuthController.h"
#include "CloudBoardsManager.h"

int main(int argc, char *argv[])
{
    //поддержка прозрачности окон на macOS и Windows
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    //создание объекта приложения
    QGuiApplication app(argc, argv);

    //установка стиля Quick Controls (для корректной кастомизации интерфейса)
    //позваляет отказаться от системных стилей для элементов интерфейса
    QQuickStyle::setStyle("Basic");

    //загрузка пользовательских настроек
    SettingsManager::instance().loadSettings();

    //создание QML движка
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("ThemeManager", &ThemeManager::instance());
    engine.rootContext()->setContextProperty("SettingsManager", &SettingsManager::instance());
    engine.rootContext()->setContextProperty("ModelsManager", &ModelsManager::instance());
    engine.rootContext()->setContextProperty("AuthController", &AuthController::instance());
    engine.rootContext()->setContextProperty("CloudBoardsManager", &CloudBoardsManager::instance());
    ThemeManager::instance().applyTheme(SettingsManager::instance().getThemeName());

    //загрузка главного QML файла из модуля Qt6
    const QUrl url(QStringLiteral("qrc:/ImagoRef/src/qml/Main.qml"));
    
    //URL для загрузки главного QML файла (позволит корректно завершить работу приложения при неудачном обращении к Main.qml)
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    
    //регистрация провайдера изображений для загрузки из памяти
    engine.addImageProvider("imago", new ImagoImageProvider());
    
    engine.load(url);
    
    //проверка, что загруженные объекты не пустые
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    //запуск бесконечного цикла обработки событий пока программа не закрыта
    return app.exec();
}
