//FileController — контроллер файловых операций. Отвечает за создание, открытие и сохранение досок (.iref)

#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

class ImagoImageModel;

class FileController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("FileController is only available via BoardController.fileController")

    //свойство проверки наличия файловго пути (новыя доска или открытый файл)
    Q_PROPERTY(QString currentFilePath READ getCurrentFilePath NOTIFY filePathChanged)
    //свойство генерации заголовка окна в зависимости от файла
    Q_PROPERTY(QString windowTitle READ getWindowTitle NOTIFY filePathChanged)

public:
    explicit FileController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    //геттеры
    QString getCurrentFilePath() const;
    QString getWindowTitle() const;

    //методы файловых операций
    Q_INVOKABLE void newBoard();
    Q_INVOKABLE bool openBoard(const QUrl &fileUrl);
    Q_INVOKABLE bool saveBoard();
    Q_INVOKABLE bool saveBoardAs(const QUrl &fileUrl);

    //сеттеры
    void setGridSize(int gridSize);
    
signals:
    //сигналы для обновления интерфейса
    void filePathChanged();
    void boardLoaded();
    void boardSaved();

    //вызывается при загрузке доски с сохранённым gridSize
    void gridSizeLoaded(int gridSize);
    void cameraLoaded(qreal x, qreal y, qreal zoom);

private:
    //внутренние поля класса
    ImagoImageModel *m_model;
    QUndoStack *m_undoStack;
    QString m_currentFilePath;
    int m_gridSize = 25;
};
