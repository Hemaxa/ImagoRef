#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

class ImageItemModel;

/**
 * @brief FileController — контроллер файловых операций.
 * Отвечает за создание, открытие и сохранение досок (.iref).
 */
class FileController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("FileController is only available via BoardController.fileController")

    Q_PROPERTY(QString currentFilePath READ currentFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY filePathChanged)

public:
    explicit FileController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    QString currentFilePath() const;
    QString windowTitle() const;

    Q_INVOKABLE void newBoard();
    Q_INVOKABLE bool openBoard(const QUrl &fileUrl);
    Q_INVOKABLE bool saveBoard();
    Q_INVOKABLE bool saveBoardAs(const QUrl &fileUrl);

    /// Устанавливает gridSize для записи в файл при сохранении
    void setGridSize(int gridSize);
    
signals:
    void filePathChanged();
    void boardLoaded();
    void boardSaved();
    /// Вызывается при загрузке доски с сохранённым gridSize
    void gridSizeLoaded(int gridSize);

private:
    ImageItemModel *m_model;
    QUndoStack *m_undoStack;
    QString m_currentFilePath;
    int m_gridSize = 25;
};
