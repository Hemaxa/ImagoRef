//main.cpp - точка входа для приложения, инициализирует QML движок и регистрирует C++ типы

#include <QGuiApplication> //основной класс для GUI-приложений без виджетов
#include <QQmlApplicationEngine> //движок, который загружает и управляет QML
#include <QQmlContext> //позволяет передавать данные из C++ в QML
#include <QQuickStyle> //стили для QML
#include <QIcon> //иконки для QML

#include "SettingsManager.h"
#include "ThemeManager.h"
#include "BoardController.h"
#include "ImageItemModel.h"

int main(int argc, char *argv[])
{
    //создание объекта приложения
    QGuiApplication app(argc, argv);
    
    //установка информации о приложении
    app.setOrganizationName("ImagoRef");
    app.setApplicationName("ImagoRef");
    app.setApplicationVersion("1.0");
    app.setWindowIcon(QIcon(":/app-icon/app-icon/icon.icns"));
    
    // Установка стиля Quick Controls (для корректной кастомизации)
    QQuickStyle::setStyle("Basic");

    // Загрузка настроек
    SettingsManager::instance().loadSettings();

    // Применение начальной темы
    ThemeManager::instance().applyTheme(SettingsManager::instance().themeName());

    // Создание QML движка
    QQmlApplicationEngine engine;

    // Регистрация контекстных свойств (синглтоны)
    engine.rootContext()->setContextProperty("Settings", &SettingsManager::instance());
    engine.rootContext()->setContextProperty("Theme", &ThemeManager::instance());

    // Загрузка главного QML файла из модуля Qt6
    const QUrl url(QStringLiteral("qrc:/ImagoRef/src/qml/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    
    engine.load(url);
    
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
