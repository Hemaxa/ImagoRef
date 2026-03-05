#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

#include "ImageModel.h"
#include "FileController.h"
#include "SelectionController.h"
#include "ClipboardController.h"
#include "ToolController.h"

/**
 * @brief BoardController — фасад-контроллер приложения для QML.
 * Координирует подконтроллеры и хранит общие ресурсы (модель, undo-стек).
 */
class BoardController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ImageItemModel* model READ model CONSTANT)
    
    // Подконтроллеры
    Q_PROPERTY(FileController* fileController READ fileController CONSTANT)
    Q_PROPERTY(SelectionController* selectionController READ selectionController CONSTANT)
    Q_PROPERTY(ClipboardController* clipboardController READ clipboardController CONSTANT)
    Q_PROPERTY(ToolController* toolController READ toolController CONSTANT)

    // Состояния для кнопок Undo/Redo
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY undoStateChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY redoStateChanged)
    // Размер сетки (в пикселях)
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)

public:
    explicit BoardController(QObject *parent = nullptr);
    ~BoardController();

    ImageItemModel* model() const;
    FileController* fileController() const;
    SelectionController* selectionController() const;
    ClipboardController* clipboardController() const;
    ToolController* toolController() const;

    bool canUndo() const;
    bool canRedo() const;
    int gridSize() const;
    void setGridSize(int size);

    // Undo/Redo
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    // Отслеживание перемещения/ресайза для Undo
    Q_INVOKABLE void beginMove(int index);
    Q_INVOKABLE void endMove(int index, qreal newX, qreal newY);
    Q_INVOKABLE void beginResize(int index);
    Q_INVOKABLE void endResize(int index, qreal newX, qreal newY, qreal newWidth, qreal newHeight);

signals:
    void undoStateChanged();
    void redoStateChanged();
    void gridSizeChanged();

private:
    void connectUndoSignals();

    ImageItemModel *m_model;
    QUndoStack *m_undoStack;
    int m_gridSize;

    FileController *m_fileController;
    SelectionController *m_selectionController;
    ClipboardController *m_clipboardController;
    ToolController *m_toolController;

    // Для отслеживания начальных позиций при перемещении/ресайзе
    QPointF m_moveStartPos;
    QRectF m_resizeStartRect;
    QPointF m_resizeStartPos;
};
