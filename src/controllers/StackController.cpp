#include "StackController.h"
#include "ImageModel.h"

// ============ AddImageCommand ============

AddImageCommand::AddImageCommand(ImageItemModel *model, const QString &imageId,
                                 const QUrl &source, qreal x, qreal y, qreal w, qreal h,
                                 QUndoCommand *parent)
    : QUndoCommand("Добавление изображения", parent)
    , m_model(model)
    , m_imageId(imageId)
{
    // Initialize data with basic information, we expect the controller to have already
    // added it to the model. We can fetch it on the first redo.
    m_data.id = imageId;
    m_data.source = source;
    m_data.x = x;
    m_data.y = y;
    m_data.width = w;
    m_data.height = h;
}

void AddImageCommand::undo()
{
    m_model->removeById(m_imageId);
}

void AddImageCommand::redo()
{
    if (m_firstRedo) {
        m_firstRedo = false;
        // On first redo (creation), grab the full image data from the model so we have
        // the QPixmap, crop settings, and label saved for future redo's.
        int idx = m_model->indexById(m_imageId);
        if (idx >= 0) {
            m_data = m_model->getItem(idx);
        }
        return; // Изображение уже добавлено при создании команды
    }
    
    m_model->addImage(m_data);
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
        m_snapshots.append(item);
    }
}

void RemoveImageCommand::undo()
{
    // Восстанавливаем в обратном порядке (чтобы индексы были правильными)
    for (int i = m_snapshots.count() - 1; i >= 0; --i) {
        m_model->addImage(m_snapshots[i]);
    }
}

void RemoveImageCommand::redo()
{
    for (const ImageData &snap : m_snapshots) {
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

// ============ CropImageCommand ============

CropImageCommand::CropImageCommand(ImageItemModel *model, int index,
                                   const QPointF &oldPos, const QSizeF &oldSize, const QRectF &oldCrop,
                                   const QPointF &newPos, const QSizeF &newSize, const QRectF &newCrop,
                                   QUndoCommand *parent)
    : QUndoCommand("Обрезка изображения", parent)
    , m_model(model)
    , m_index(index)
    , m_oldPos(oldPos), m_newPos(newPos)
    , m_oldSize(oldSize), m_newSize(newSize)
    , m_oldCrop(oldCrop), m_newCrop(newCrop)
{
}

void CropImageCommand::undo()
{
    m_model->updatePosition(m_index, m_oldPos.x(), m_oldPos.y());
    m_model->updateSize(m_index, m_oldSize.width(), m_oldSize.height());
    m_model->updateCrop(m_index, m_oldCrop.x(), m_oldCrop.y(), m_oldCrop.width(), m_oldCrop.height());
}

void CropImageCommand::redo()
{
    m_model->updatePosition(m_index, m_newPos.x(), m_newPos.y());
    m_model->updateSize(m_index, m_newSize.width(), m_newSize.height());
    m_model->updateCrop(m_index, m_newCrop.x(), m_newCrop.y(), m_newCrop.width(), m_newCrop.height());
}

// ============ SetLabelCommand ============

SetLabelCommand::SetLabelCommand(ImageItemModel *model, int index,
                                 const QString &oldLabel, const QString &newLabel,
                                 QUndoCommand *parent)
    : QUndoCommand("Изменение подписи", parent)
    , m_model(model)
    , m_index(index)
    , m_oldLabel(oldLabel)
    , m_newLabel(newLabel)
{
}

void SetLabelCommand::undo()
{
    m_model->updateLabel(m_index, m_oldLabel);
}

void SetLabelCommand::redo()
{
    m_model->updateLabel(m_index, m_newLabel);
}

// ============ ArrangeCommand ============

ArrangeCommand::ArrangeCommand(ImageItemModel *model,
                               const QVector<int> &indices,
                               const QVector<QPointF> &oldPositions,
                               const QVector<QPointF> &newPositions,
                               QUndoCommand *parent)
    : QUndoCommand("Расположить изображения", parent)
    , m_model(model)
    , m_indices(indices)
    , m_oldPositions(oldPositions)
    , m_newPositions(newPositions)
{
}

void ArrangeCommand::undo()
{
    for (int i = 0; i < m_indices.count(); ++i) {
        m_model->updatePosition(m_indices[i], m_oldPositions[i].x(), m_oldPositions[i].y());
    }
}

void ArrangeCommand::redo()
{
    for (int i = 0; i < m_indices.count(); ++i) {
        m_model->updatePosition(m_indices[i], m_newPositions[i].x(), m_newPositions[i].y());
    }
}

// ============ UpscaleImageCommand ============

UpscaleImageCommand::UpscaleImageCommand(ImageItemModel *model, int index,
                                         const QPixmap &oldPixmap, const QRectF &oldCrop,
                                         const QPixmap &newPixmap, const QRectF &newCrop,
                                         QUndoCommand *parent)
    : QUndoCommand("Увеличение разрешения", parent)
    , m_model(model)
    , m_index(index)
    , m_oldPixmap(oldPixmap), m_newPixmap(newPixmap)
    , m_oldCrop(oldCrop), m_newCrop(newCrop)
{
}

void UpscaleImageCommand::undo()
{
    m_model->updatePixmap(m_index, m_oldPixmap);
    m_model->updateCrop(m_index, m_oldCrop.x(), m_oldCrop.y(), m_oldCrop.width(), m_oldCrop.height());
}

void UpscaleImageCommand::redo()
{
    m_model->updatePixmap(m_index, m_newPixmap);
    m_model->updateCrop(m_index, m_newCrop.x(), m_newCrop.y(), m_newCrop.width(), m_newCrop.height());
}
