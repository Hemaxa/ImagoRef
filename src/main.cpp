//main.cpp - точка входа для приложения, инициализирует QML движок и регистрирует C++ типы

#include <QGuiApplication> //основной класс для GUI-приложений без виджетов
#include <QQmlApplicationEngine> //движок, который загружает и управляет QML
#include <QQmlContext> //позволяет передавать данные из C++ в QML
#include <QQuickStyle> //стили для QML
#include <QIcon> //иконки для QML

#include "SettingsManager.h"
#include "ThemesManager.h"

int main(int argc, char *argv[])
{
    //создание объекта приложения
    QGuiApplication app(argc, argv);
    
    //установка стиля Quick Controls (для корректной кастомизации интерфейса)
    QQuickStyle::setStyle("Basic");

    //загрузка пользовательских настроек
    SettingsManager::instance().loadSettings();
    ThemeManager::instance().applyTheme(SettingsManager::instance().themeName());

    //создание QML движка
    QQmlApplicationEngine engine;

    //регистрация контекстных свойств (синглтоны)
    engine.rootContext()->setContextProperty("Settings", &SettingsManager::instance());
    engine.rootContext()->setContextProperty("Theme", &ThemeManager::instance());

    //загрузка главного QML файла из модуля Qt6
    const QUrl url(QStringLiteral("qrc:/ImagoRef/src/qml/Main.qml"));
    
    //URL для загрузки главного QML файла
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);
    
    //проверка, что загруженные объекты не пустые
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
