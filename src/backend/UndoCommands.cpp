#include "UndoCommands.h"
#include "ImageItemModel.h"

// ============ AddImageCommand ============

AddImageCommand::AddImageCommand(ImageItemModel *model, const QString &imageId,
                                 const QUrl &source, qreal x, qreal y, qreal w, qreal h,
                                 QUndoCommand *parent)
    : QUndoCommand("Добавление изображения", parent)
    , m_model(model)
    , m_imageId(imageId)
    , m_source(source)
    , m_x(x), m_y(y), m_width(w), m_height(h)
{
}

void AddImageCommand::undo()
{
    m_model->removeById(m_imageId);
}

void AddImageCommand::redo()
{
    if (m_firstRedo) {
        m_firstRedo = false;
        return; // Изображение уже добавлено при создании команды
    }
    
    ImageData data;
    data.id = m_imageId;
    data.source = m_source;
    data.x = m_x;
    data.y = m_y;
    data.width = m_width;
    data.height = m_height;
    m_model->addImage(data);
}

// ============ RemoveImageCommand ============

RemoveImageCommand::RemoveImageCommand(ImageItemModel *model, const QList<int> &indices,
                                       QUndoCommand *parent)
    : QUndoCommand(QString("Удаление %1 элементов").arg(indices.count()), parent)
    , m_model(model)
{
    // Сохраняем снимки удаляемых элементов (в обратном порядке для корректного удаления)
    QList<int> sortedIndices = indices;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());
    
    for (int idx : sortedIndices) {
        ImageData item = m_model->getItem(idx);
        ImageSnapshot snap;
        snap.id = item.id;
        snap.source = item.source;
        snap.x = item.x;
        snap.y = item.y;
        snap.width = item.width;
        snap.height = item.height;
        snap.rotation = item.rotation;
        snap.zValue = item.zValue;
        m_snapshots.append(snap);
    }
}

void RemoveImageCommand::undo()
{
    // Восстанавливаем в обратном порядке (чтобы индексы были правильными)
    for (int i = m_snapshots.count() - 1; i >= 0; --i) {
        const ImageSnapshot &snap = m_snapshots[i];
        ImageData data;
        data.id = snap.id;
        data.source = snap.source;
        data.x = snap.x;
        data.y = snap.y;
        data.width = snap.width;
        data.height = snap.height;
        data.rotation = snap.rotation;
        data.zValue = snap.zValue;
        m_model->addImage(data);
    }
}

void RemoveImageCommand::redo()
{
    for (const ImageSnapshot &snap : m_snapshots) {
        m_model->removeById(snap.id);
    }
}

// ============ MoveImageCommand ============

MoveImageCommand::MoveImageCommand(ImageItemModel *model, int index,
                                   const QPointF &oldPos, const QPointF &newPos,
                                   QUndoCommand *parent)
    : QUndoCommand("Перемещение элемента", parent)
    , m_model(model)
    , m_index(index)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
}

void MoveImageCommand::undo()
{
    m_model->updatePosition(m_index, m_oldPos.x(), m_oldPos.y());
}

void MoveImageCommand::redo()
{
    m_model->updatePosition(m_index, m_newPos.x(), m_newPos.y());
}

bool MoveImageCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const MoveImageCommand *cmd = static_cast<const MoveImageCommand*>(other);
    if (cmd->m_index != m_index)
        return false;

    m_newPos = cmd->m_newPos;
    return true;
}

// ============ ResizeImageCommand ============

ResizeImageCommand::ResizeImageCommand(ImageItemModel *model, int index,
                                       const QRectF &oldRect, const QPointF &oldPos,
                                       const QRectF &newRect, const QPointF &newPos,
                                       QUndoCommand *parent)
    : QUndoCommand("Изменение размера", parent)
    , m_model(model)
    , m_index(index)
    , m_oldRect(oldRect), m_newRect(newRect)
    , m_oldPos(oldPos), m_newPos(newPos)
{
}

void ResizeImageCommand::undo()
{
    m_model->updatePosition(m_index, m_oldPos.x(), m_oldPos.y());
    m_model->updateSize(m_index, m_oldRect.width(), m_oldRect.height());
}

void ResizeImageCommand::redo()
{
    m_model->updatePosition(m_index, m_newPos.x(), m_newPos.y());
    m_model->updateSize(m_index, m_newRect.width(), m_newRect.height());
}

// ============ RotateImageCommand ============

RotateImageCommand::RotateImageCommand(ImageItemModel *model, int index, qreal angleDelta,
                                       QUndoCommand *parent)
    : QUndoCommand("Вращение элемента", parent)
    , m_model(model)
    , m_index(index)
    , m_angleDelta(angleDelta)
{
}

void RotateImageCommand::undo()
{
    ImageData item = m_model->getItem(m_index);
    m_model->updateRotation(m_index, item.rotation - m_angleDelta);
}

void RotateImageCommand::redo()
{
    ImageData item = m_model->getItem(m_index);
    m_model->updateRotation(m_index, item.rotation + m_angleDelta);
}
