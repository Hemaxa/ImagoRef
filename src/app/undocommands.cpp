#include "undocommands.h"

//AddCommand
AddCommand::AddCommand(ImageItem *item, QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent), m_item(item), m_scene(scene), m_isInitialAdd(true) {
    setText("Добавление изображения");
}

AddCommand::~AddCommand() {
    if (m_item && !m_item->scene()) {
        delete m_item;
    }
}

void AddCommand::undo() {
    if (m_item) m_scene->removeItem(m_item);
    m_isInitialAdd = false;
}

void AddCommand::redo() {
    if (m_item) m_scene->addItem(m_item);
    m_isInitialAdd = true;
}

//RemoveCommand
RemoveCommand::RemoveCommand(const QList<QGraphicsItem*> &items, QGraphicsScene *scene, QUndoCommand *parent)
    : QUndoCommand(parent), m_scene(scene) {
    setText(QString("Удаление %1 элементов").arg(items.count()));
    // Безопасно преобразуем указатели
    for (QGraphicsItem* item : items) {
        if (ImageItem* imageItem = dynamic_cast<ImageItem*>(item)) {
            m_items.append(imageItem);
        }
    }
}

RemoveCommand::~RemoveCommand() {
    // Теперь проверка абсолютно безопасна
    if (!m_items.isEmpty() && m_items.first() && !m_items.first()->scene()) {
        // qDeleteAll не работает с QList<QPointer>, удаляем вручную
        for (QPointer<ImageItem> item : m_items) {
            if (item) { // Удаляем только если объект еще существует
                delete item;
            }
        }
    }
}

void RemoveCommand::undo() {
    for (QPointer<ImageItem> item : m_items) {
        if (item) { // Проверяем, не был ли объект удален
            m_scene->addItem(item);
        }
    }
}

void RemoveCommand::redo() {
    for (QPointer<ImageItem> item : m_items) {
        if (item) { // Проверяем, не был ли объект удален
            m_scene->removeItem(item);
        }
    }
}

//MoveCommand
MoveCommand::MoveCommand(QGraphicsItem *item, const QPointF &oldPos, QUndoCommand *parent)
    : QUndoCommand(parent), m_oldPos(oldPos) {
    // Безопасное приведение и сохранение
    if ((m_item = dynamic_cast<ImageItem*>(item))) {
        m_newPos = m_item->pos();
    }
    setText("Перемещение элемента");
}

void MoveCommand::undo() {
    if (m_item) { // Безопасная проверка
        m_item->setPos(m_oldPos);
        if(m_item->scene()) m_item->scene()->update();
    }
}

void MoveCommand::redo() {
    if (m_item) { // Безопасная проверка
        m_item->setPos(m_newPos);
        if(m_item->scene()) m_item->scene()->update();
    }
}

//ResizeCommand
ResizeCommand::ResizeCommand(ImageItem *item, const QRectF &oldRect, const QPointF &oldPos, QUndoCommand *parent)
    : QUndoCommand(parent), m_item(item), m_oldRect(oldRect), m_oldPos(oldPos) {
    if (m_item) {
        m_newRect = m_item->boundingRect();
        m_newPos = m_item->pos();
    }
    setText("Изменение размера");
}

void ResizeCommand::undo() {
    if (m_item) m_item->setGeometry(m_oldRect, m_oldPos); // Безопасная проверка
}

void ResizeCommand::redo() {
    if (m_item) m_item->setGeometry(m_newRect, m_newPos); // Безопасная проверка
}
