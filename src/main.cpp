#include "WelcomeWindow.h"
#include "MainWindow.h"
#include "SettingsManager.h"
#include "ThemeManager.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Загружаем настройки (тему, размер сетки)
    SettingsManager::instance().loadSettings();

    // Применяем тему до создания виджетов
    ThemeManager::instance().applyTheme(SettingsManager::instance().getThemeName());

    WelcomeWindow welcomeWindow;
    MainWindow mainWindow;

    mainWindow.applyInitialSettings();

    QObject::connect(&welcomeWindow, &WelcomeWindow::newBoardRequested, [&]() {
        mainWindow.show();
        welcomeWindow.accept();
    });

    QObject::connect(&welcomeWindow, &WelcomeWindow::openBoardRequested, [&]() {
        if (mainWindow.openBoard())
        {
            mainWindow.show();
            welcomeWindow.accept();
        }
    });

    if (welcomeWindow.exec() == QDialog::Accepted)
    {
        return app.exec();
    }
    else
    {
        return 0;
    }
}
