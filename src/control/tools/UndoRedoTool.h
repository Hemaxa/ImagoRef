#pragma once

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QPointF>
#include <QRectF>
#include <QPointer>

// Прямое объявление, чтобы избежать циклической зависимости
class ImageItem;

// Команда для добавления нового элемента на сцену
class AddCommand : public QUndoCommand {
public:
    AddCommand(ImageItem *item, QGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~AddCommand();
    void undo() override;
    void redo() override;
private:
    QPointer<ImageItem> m_item;
    QGraphicsScene *m_scene;
    bool m_isInitialAdd;
};

// Команда для удаления элементов со сцены
class RemoveCommand : public QUndoCommand {
public:
    RemoveCommand(const QList<QGraphicsItem*> &items, QGraphicsScene *scene, QUndoCommand *parent = nullptr);
    ~RemoveCommand();
    void undo() override;
    void redo() override;
private:
    QList<QPointer<ImageItem>> m_items;
    QGraphicsScene *m_scene;
};

// Команда для перемещения элемента
class MoveCommand : public QUndoCommand {
public:
    MoveCommand(QGraphicsItem *item, const QPointF &oldPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<ImageItem> m_item;
    QPointF m_oldPos;
    QPointF m_newPos;
};

// Команда для изменения размера элемента
class ResizeCommand : public QUndoCommand {
public:
    ResizeCommand(ImageItem *item, const QRectF &oldRect, const QPointF &oldPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<ImageItem> m_item;
    QRectF m_oldRect;
    QRectF m_newRect;
    QPointF m_oldPos;
    QPointF m_newPos;
};

// Команда для изменения угла поворота элемента
class RotateCommand : public QUndoCommand {
public:
    explicit RotateCommand(ImageItem *item, qreal angleDelta, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    QPointer<ImageItem> m_item;
    qreal m_angleDelta;
};
