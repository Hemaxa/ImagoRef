#include "WelcomeWindow.h" // ✅
#include "MainWindow.h"    // ✅

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QFile file(":/themes/themes/imago.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(file.readAll());
        file.close();
    }

    WelcomeWindow welcomeWindow;
    MainWindow mainWindow;

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
