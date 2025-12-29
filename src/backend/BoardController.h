#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

#include "ImageItemModel.h"

/**
 * @brief BoardController - главный контроллер приложения для QML.
 * Управляет холстом, файловыми операциями и инструментами.
 */
class BoardController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ImageItemModel* model READ model CONSTANT)
    // Текущий путь к файлу доски
    Q_PROPERTY(QString currentFilePath READ currentFilePath NOTIFY filePathChanged)
    // Заголовок окна (зависит от имени файла)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY filePathChanged)
    // Состояния для кнопок Undo/Redo
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY redoStateChanged)
    // Размер сетки (в пикселях)
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    // Есть ли выделенные элементы (для активации кнопок инструментов)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)

public:
    explicit BoardController(QObject *parent = nullptr);
    ~BoardController();

    ImageItemModel* model() const;
    QString currentFilePath() const;
    QString windowTitle() const;
    bool canUndo() const;
    bool canRedo() const;
    int gridSize() const;
    void setGridSize(int size);
    bool hasSelection() const;

    // Файловые операции
    Q_INVOKABLE bool openBoard(const QUrl &fileUrl);
    Q_INVOKABLE bool saveBoard();
    Q_INVOKABLE bool saveBoardAs(const QUrl &fileUrl);
    Q_INVOKABLE void newBoard();

    // Операции с изображениями
    Q_INVOKABLE void addImage(const QUrl &imageUrl, qreal x, qreal y);
    Q_INVOKABLE void addImageFromPixmap(const QByteArray &imageData, qreal x, qreal y);
    Q_INVOKABLE void pasteFromClipboard(qreal x, qreal y);
    
    // Выделение
    Q_INVOKABLE void selectItem(int index, bool addToSelection = false);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();

    // Инструменты
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void snapToGrid();
    Q_INVOKABLE void rotateSelected(qreal angleDelta);

    // Undo/Redo
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    // Методы для отслеживания начала и конца перемещения/изменения размера.
    // Необходимы для создания одной команды Undo после завершения интерактивного действия в QML.
    Q_INVOKABLE void beginMove(int index);
    Q_INVOKABLE void endMove(int index, qreal newX, qreal newY);
    Q_INVOKABLE void beginResize(int index);
    Q_INVOKABLE void endResize(int index, qreal newX, qreal newY, qreal newWidth, qreal newHeight);

signals:
    void filePathChanged();
    void undoStateChanged();
    void redoStateChanged();
    void gridSizeChanged();
    void selectionChanged();
    void boardLoaded();
    void boardSaved();

private:
    void updateWindowTitle();
    void connectUndoSignals();

    ImageItemModel *m_model;
    QUndoStack *m_undoStack;
    QString m_currentFilePath;
    int m_gridSize;

    // Для отслеживания начальных позиций при перемещении/ресайзе
    QPointF m_moveStartPos;
    QRectF m_resizeStartRect;
    QPointF m_resizeStartPos;
};
