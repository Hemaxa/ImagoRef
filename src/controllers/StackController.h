#pragma once

#include <QUndoCommand>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QUrl>

#include "ImageModel.h"

/**
 * @brief AddImageCommand - команда добавления изображения на холст.
 */
class AddImageCommand : public QUndoCommand {
public:
    AddImageCommand(ImageItemModel *model, const QString &imageId, 
                    const QUrl &source, qreal x, qreal y, qreal w, qreal h,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    QString m_imageId;
    ImageData m_data; // Store full image data for redo
    bool m_firstRedo = true;
};

/**
 * @brief RemoveImageCommand - команда удаления изображений с холста.
 */
class RemoveImageCommand : public QUndoCommand {
public:
    RemoveImageCommand(ImageItemModel *model, const QList<int> &indices,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    QList<ImageData> m_snapshots; // Store full ImageData to preserve pixmap, crop, label etc.
};

/**
 * @brief MoveImageCommand - команда перемещения изображения.
 */
class MoveImageCommand : public QUndoCommand {
public:
    MoveImageCommand(ImageItemModel *model, int index,
                     const QPointF &oldPos, const QPointF &newPos,
                     QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    ImageItemModel *m_model;
    int m_index;
    QPointF m_oldPos;
    QPointF m_newPos;
};

/**
 * @brief ResizeImageCommand - команда изменения размера изображения.
 */
class ResizeImageCommand : public QUndoCommand {
public:
    ResizeImageCommand(ImageItemModel *model, int index,
                       const QRectF &oldRect, const QPointF &oldPos,
                       const QRectF &newRect, const QPointF &newPos,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    int m_index;
    QRectF m_oldRect, m_newRect;
    QPointF m_oldPos, m_newPos;
};

/**
 * @brief RotateImageCommand - команда вращения изображения.
 */
class RotateImageCommand : public QUndoCommand {
public:
    RotateImageCommand(ImageItemModel *model, int index, qreal angleDelta,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    int m_index;
    qreal m_angleDelta;
};

/**
 * @brief CropImageCommand - команда обрезки изображения.
 * Сохраняет позицию, размер и crop rect для undo/redo.
 */
class CropImageCommand : public QUndoCommand {
public:
    CropImageCommand(ImageItemModel *model, int index,
                     const QPointF &oldPos, const QSizeF &oldSize, const QRectF &oldCrop,
                     const QPointF &newPos, const QSizeF &newSize, const QRectF &newCrop,
                     QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    int m_index;
    QPointF m_oldPos, m_newPos;
    QSizeF m_oldSize, m_newSize;
    QRectF m_oldCrop, m_newCrop;
};

/**
 * @brief SetLabelCommand - команда установки подписи изображения.
 */
class SetLabelCommand : public QUndoCommand {
public:
    SetLabelCommand(ImageItemModel *model, int index,
                    const QString &oldLabel, const QString &newLabel,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    int m_index;
    QString m_oldLabel, m_newLabel;
};

/**
 * @brief ArrangeCommand - команда автоматического расположения изображений.
 * Сохраняет старые и новые позиции для undo/redo.
 */
class ArrangeCommand : public QUndoCommand {
public:
    ArrangeCommand(ImageItemModel *model,
                   const QVector<int> &indices,
                   const QVector<QPointF> &oldPositions,
                   const QVector<QPointF> &newPositions,
                   QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    QVector<int> m_indices;
    QVector<QPointF> m_oldPositions, m_newPositions;
};

/**
 * @brief UpscaleImageCommand - команда применения/отмены увеличения разрешения.
 */
class UpscaleImageCommand : public QUndoCommand {
public:
    UpscaleImageCommand(ImageItemModel *model, int index,
                        const QPixmap &oldPixmap, const QRectF &oldCrop,
                        const QPixmap &newPixmap, const QRectF &newCrop,
                        QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    int m_index;
    QPixmap m_oldPixmap, m_newPixmap;
    QRectF m_oldCrop, m_newCrop;
};
