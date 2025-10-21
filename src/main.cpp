//Main - входная точка приложения. Инициализирует компоненты в правильном порядке.

#include "WelcomeWindow.h"
#include "MainWindow.h"
#include "SettingsManager.h"
#include "ThemeManager.h"

#include <QApplication> //класс приложения Qt
#include <QFile>

int main(int argc, char *argv[]) {
    //создание главного класса приложения Qt
    QApplication app(argc, argv);

    //загрузка настроек приложения
    SettingsManager::instance().loadSettings();

    //применение темы
    ThemeManager::instance().applyTheme(SettingsManager::instance().getThemeName());

    //создание экземпляров стартового и главного окон
    WelcomeWindow welcomeWindow;
    MainWindow mainWindow;

    //применение настроек приложения
    mainWindow.applyInitialSettings();

    //создание нового окна приложения
    QObject::connect(&welcomeWindow, &WelcomeWindow::newBoardRequested, [&]() {
        mainWindow.show();
        welcomeWindow.accept();
    });

    //открытие уже существующей доски
    QObject::connect(&welcomeWindow, &WelcomeWindow::openBoardRequested, [&]() {
        if (mainWindow.openBoard())
        {
            mainWindow.show();
            welcomeWindow.accept();
        }
    });

    //запуск стартового окна в режиме модального диалога
    if (welcomeWindow.exec() == QDialog::Accepted)
    {
        return app.exec();
    }
    else
    {
        return 0;
    }
}
