// src/main.cpp
#include <QApplication>
#include <QMainWindow> // Временно для проверки, потом заменим на наш класс

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Позже здесь будет создание нашего главного окна ImagoRef
    QMainWindow window;
    window.setWindowTitle("ImagoRef");
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
