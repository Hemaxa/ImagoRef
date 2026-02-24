#pragma once

#include <QUndoCommand>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QUrl>

class ImageItemModel;

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
    QUrl m_source;
    qreal m_x, m_y, m_width, m_height;
    bool m_firstRedo = true;
};

/**
 * @brief RemoveImageCommand - команда удаления изображений с холста.
 */
class RemoveImageCommand : public QUndoCommand {
public:
    struct ImageSnapshot {
        QString id;
        QUrl source;
        qreal x, y, width, height, rotation, zValue;
    };

    RemoveImageCommand(ImageItemModel *model, const QList<int> &indices,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImageItemModel *m_model;
    QList<ImageSnapshot> m_snapshots;
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
