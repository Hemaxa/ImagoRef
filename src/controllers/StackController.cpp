#include "StackController.h"
#include "ImageModel.h"

AddImageCommand::AddImageCommand(ImagoImageModel *model, const QString &imageId, const QUrl &source, qreal x, qreal y, qreal w, qreal h, QUndoCommand *parent) : QUndoCommand("Добавление изображения", parent)
    , m_model(model)
    , m_imageId(imageId)
{
    m_data.id = imageId;
    m_data.source = source;
    m_data.x = x;
    m_data.y = y;
    m_data.width = w;
    m_data.height = h;
}

void AddImageCommand::undo()
{
    m_model->removeImageById(m_imageId);
}

void AddImageCommand::redo()
{
    if (m_firstRedo) {
        m_firstRedo = false;
        int idx = m_model->getIndexById(m_imageId);
        if (idx >= 0) {
            m_data = m_model->getItem(idx);
        }
        return; //изображение уже добавлено при создании команды
    }
    
    m_model->addImage(m_data);
}

RemoveImageCommand::RemoveImageCommand(ImagoImageModel *model, const QList<int> &indices, QUndoCommand *parent) : QUndoCommand(QString("Удаление %1 элементов").arg(indices.count()), parent)
    , m_model(model)
{
    //сохраняем снимки удаляемых элементов (в обратном порядке для корректного удаления)
    QList<int> sortedIndices = indices;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());
    
    for (int idx : sortedIndices) {
        ImagoImageData item = m_model->getItem(idx);
        m_snapshots.append(item);
    }
}

void RemoveImageCommand::undo()
{
    //восстанавливаем в обратном порядке (чтобы индексы были правильными)
    for (int i = m_snapshots.count() - 1; i >= 0; --i) {
        m_model->addImage(m_snapshots[i]);
    }
}

void RemoveImageCommand::redo()
{
    for (const ImagoImageData &snap : m_snapshots) {
        m_model->removeImageById(snap.id);
    }
}

MoveImageCommand::MoveImageCommand(ImagoImageModel *model, int index, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent) : QUndoCommand("Перемещение элемента", parent)
    , m_model(model)
    , m_index(index)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
}

void MoveImageCommand::undo()
{
    m_model->setPosition(m_index, m_oldPos.x(), m_oldPos.y());
}

void MoveImageCommand::redo()
{
    m_model->setPosition(m_index, m_newPos.x(), m_newPos.y());
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

MoveImagesCommand::MoveImagesCommand(ImagoImageModel *model, const QVector<int>& indices, const QVector<QPointF> &oldPos, const QVector<QPointF> &newPos, QUndoCommand *parent) : QUndoCommand("Перемещение элементов", parent)
    , m_model(model)
    , m_indices(indices)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
}

void MoveImagesCommand::undo()
{
    for (int i = 0; i < m_indices.size(); ++i) {
        m_model->setPosition(m_indices[i], m_oldPos[i].x(), m_oldPos[i].y());
    }
}

void MoveImagesCommand::redo()
{
    for (int i = 0; i < m_indices.size(); ++i) {
        m_model->setPosition(m_indices[i], m_newPos[i].x(), m_newPos[i].y());
    }
}

bool MoveImagesCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const MoveImagesCommand *cmd = static_cast<const MoveImagesCommand*>(other);
    if (cmd->m_indices != m_indices) // Must be exactly the same subset of indices
        return false;

    m_newPos = cmd->m_newPos;
    return true;
}

ResizeImageCommand::ResizeImageCommand(ImagoImageModel *model, int index, const QRectF &oldRect, const QPointF &oldPos, const QRectF &newRect, const QPointF &newPos, QUndoCommand *parent) : QUndoCommand("Изменение размера", parent)
    , m_model(model)
    , m_index(index)
    , m_oldRect(oldRect), m_newRect(newRect)
    , m_oldPos(oldPos), m_newPos(newPos)
{
}

void ResizeImageCommand::undo()
{
    m_model->setPosition(m_index, m_oldPos.x(), m_oldPos.y());
    m_model->setSize(m_index, m_oldRect.width(), m_oldRect.height());
}

void ResizeImageCommand::redo()
{
    m_model->setPosition(m_index, m_newPos.x(), m_newPos.y());
    m_model->setSize(m_index, m_newRect.width(), m_newRect.height());
}

RotateImageCommand::RotateImageCommand(ImagoImageModel *model, int index, qreal angleDelta, QUndoCommand *parent) : QUndoCommand("Вращение элемента", parent)
    , m_model(model)
    , m_index(index)
    , m_angleDelta(angleDelta)
{
}

void RotateImageCommand::undo()
{
    ImagoImageData item = m_model->getItem(m_index);
    m_model->setRotation(m_index, item.rotation - m_angleDelta);
}

void RotateImageCommand::redo()
{
    ImagoImageData item = m_model->getItem(m_index);
    m_model->setRotation(m_index, item.rotation + m_angleDelta);
}

CropImageCommand::CropImageCommand(ImagoImageModel *model, int index, const QPointF &oldPos, const QSizeF &oldSize, const QRectF &oldCrop, const QPointF &newPos, const QSizeF &newSize, const QRectF &newCrop, QUndoCommand *parent) : QUndoCommand("Обрезка изображения", parent)
    , m_model(model)
    , m_index(index)
    , m_oldPos(oldPos), m_newPos(newPos)
    , m_oldSize(oldSize), m_newSize(newSize)
    , m_oldCrop(oldCrop), m_newCrop(newCrop)
{
}

void CropImageCommand::undo()
{
    m_model->setPosition(m_index, m_oldPos.x(), m_oldPos.y());
    m_model->setSize(m_index, m_oldSize.width(), m_oldSize.height());
    m_model->setCrop(m_index, m_oldCrop.x(), m_oldCrop.y(), m_oldCrop.width(), m_oldCrop.height());
}

void CropImageCommand::redo()
{
    m_model->setPosition(m_index, m_newPos.x(), m_newPos.y());
    m_model->setSize(m_index, m_newSize.width(), m_newSize.height());
    m_model->setCrop(m_index, m_newCrop.x(), m_newCrop.y(), m_newCrop.width(), m_newCrop.height());
}

SetLabelCommand::SetLabelCommand(ImagoImageModel *model, int index, const QString &oldLabel, const QString &newLabel, QUndoCommand *parent) : QUndoCommand("Изменение подписи", parent)
    , m_model(model)
    , m_index(index)
    , m_oldLabel(oldLabel)
    , m_newLabel(newLabel)
{
}

void SetLabelCommand::undo()
{
    m_model->setLabel(m_index, m_oldLabel);
}

void SetLabelCommand::redo()
{
    m_model->setLabel(m_index, m_newLabel);
}

SetOpacityCommand::SetOpacityCommand(ImagoImageModel *model, const QVector<int> &indices, const QVector<qreal> &oldOpacities, qreal newOpacity, QUndoCommand *parent) : QUndoCommand("Изменение непрозрачности", parent)
    , m_model(model)
    , m_indices(indices)
    , m_oldOpacities(oldOpacities)
    , m_newOpacity(newOpacity)
{
}

void SetOpacityCommand::undo()
{
    for (int i = 0; i < m_indices.count(); ++i) {
        m_model->setOpacity(m_indices[i], m_oldOpacities[i]);
    }
}

void SetOpacityCommand::redo()
{
    for (int i = 0; i < m_indices.count(); ++i) {
        m_model->setOpacity(m_indices[i], m_newOpacity);
    }
}

bool SetOpacityCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const SetOpacityCommand *cmd = static_cast<const SetOpacityCommand*>(other);
    if (cmd->m_indices != m_indices) // Must be exactly the same subset of indices
        return false;

    m_newOpacity = cmd->m_newOpacity;
    return true;
}

ArrangeCommand::ArrangeCommand(ImagoImageModel *model, const QVector<int> &indices, const QVector<QPointF> &oldPositions, const QVector<QPointF> &newPositions, QUndoCommand *parent) : QUndoCommand("Расположить изображения", parent)
    , m_model(model)
    , m_indices(indices)
    , m_oldPositions(oldPositions)
    , m_newPositions(newPositions)
{
}

void ArrangeCommand::undo()
{
    for (int i = 0; i < m_indices.count(); ++i) {
        m_model->setPosition(m_indices[i], m_oldPositions[i].x(), m_oldPositions[i].y());
    }
}

void ArrangeCommand::redo()
{
    for (int i = 0; i < m_indices.count(); ++i) {
        m_model->setPosition(m_indices[i], m_newPositions[i].x(), m_newPositions[i].y());
    }
}

UpscaleImageCommand::UpscaleImageCommand(ImagoImageModel *model, int index, const QPixmap &oldPix, const QRectF &oldCrop, const QString &oldHash, const QPixmap &newPix, const QRectF &newCrop, const QString &newHash, QUndoCommand *parent) : QUndoCommand(parent)
    , m_model(model)
    , m_index(index)
    , m_oldPix(oldPix)
    , m_newPix(newPix)
    , m_oldCrop(oldCrop)
    , m_newCrop(newCrop)
    , m_oldHash(oldHash)
    , m_newHash(newHash)
{
}

void UpscaleImageCommand::undo() {
    m_model->setPixmap(m_index, m_oldPix);
    m_model->setImageHash(m_index, m_oldHash); // Восстанавливаем старый хэш
    m_model->setCrop(m_index, m_oldCrop.x(), m_oldCrop.y(), m_oldCrop.width(), m_oldCrop.height());
}

void UpscaleImageCommand::redo() {
    m_model->setPixmap(m_index, m_newPix);
    m_model->setImageHash(m_index, m_newHash); // Применяем новый хэш
    m_model->setCrop(m_index, m_newCrop.x(), m_newCrop.y(), m_newCrop.width(), m_newCrop.height());
}
