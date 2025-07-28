#include "mainwindow.h"
#include <QApplication> //класс приложения Qt

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window; //создание экземпляра главного окна приложеня

    window.show();

    //главный цикл обработки событий
    return app.exec();
}
