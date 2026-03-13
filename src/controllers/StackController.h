//StackController — контроллер системы отмены действий (Undo/Redo)

#pragma once

#include <QUndoCommand>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QUrl>

#include "ImageModel.h"

//AddImageCommand - команда добавления изображения на холст
class AddImageCommand : public QUndoCommand {
public:
    AddImageCommand(ImagoImageModel *model, const QString &imageId, const QUrl &source, qreal x, qreal y, qreal w, qreal h, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    QString m_imageId;
    ImagoImageData m_data;
    bool m_firstRedo = true;
};

//RemoveImageCommand - команда удаления изображений с холста
class RemoveImageCommand : public QUndoCommand {
public:
    RemoveImageCommand(ImagoImageModel *model, const QList<int> &indices, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    QList<ImagoImageData> m_snapshots;
};

//MoveImageCommand - команда перемещения изображения
class MoveImageCommand : public QUndoCommand {
public:
    MoveImageCommand(ImagoImageModel *model, int index, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    ImagoImageModel *m_model;
    int m_index;
    QPointF m_oldPos;
    QPointF m_newPos;
};

//MoveImagesCommand - команда перемещения нескольких изображений одновременно
class MoveImagesCommand : public QUndoCommand {
public:
    MoveImagesCommand(ImagoImageModel *model, const QVector<int>& indices, const QVector<QPointF> &oldPos, const QVector<QPointF> &newPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1002; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    ImagoImageModel *m_model;
    QVector<int> m_indices;
    QVector<QPointF> m_oldPos;
    QVector<QPointF> m_newPos;
};

//ResizeImageCommand - команда изменения размера изображения
class ResizeImageCommand : public QUndoCommand {
public:
    ResizeImageCommand(ImagoImageModel *model, int index, const QRectF &oldRect, const QPointF &oldPos, const QRectF &newRect, const QPointF &newPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    int m_index;
    QRectF m_oldRect, m_newRect;
    QPointF m_oldPos, m_newPos;
};

//RotateImageCommand - команда вращения изображения
class RotateImageCommand : public QUndoCommand {
public:
    RotateImageCommand(ImagoImageModel *model, int index, qreal angleDelta, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    int m_index;
    qreal m_angleDelta;
};

//CropImageCommand - команда обрезки изображения. Сохраняет позицию, размер и crop rect для undo/redo
class CropImageCommand : public QUndoCommand {
public:
    CropImageCommand(ImagoImageModel *model, int index, const QPointF &oldPos, const QSizeF &oldSize, const QRectF &oldCrop, const QPointF &newPos, const QSizeF &newSize, const QRectF &newCrop, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    int m_index;
    QPointF m_oldPos, m_newPos;
    QSizeF m_oldSize, m_newSize;
    QRectF m_oldCrop, m_newCrop;
};

//SetLabelCommand - команда установки подписи изображения
class SetLabelCommand : public QUndoCommand {
public:
    SetLabelCommand(ImagoImageModel *model, int index, const QString &oldLabel, const QString &newLabel, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    int m_index;
    QString m_oldLabel, m_newLabel;
};

//ArrangeCommand - команда автоматического расположения изображений. Сохраняет старые и новые позиции для undo/redo
class ArrangeCommand : public QUndoCommand {
public:
    ArrangeCommand(ImagoImageModel *model, const QVector<int> &indices, const QVector<QPointF> &oldPositions, const QVector<QPointF> &newPositions, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    QVector<int> m_indices;
    QVector<QPointF> m_oldPositions, m_newPositions;
};

//UpscaleImageCommand - команда применения/отмены увеличения разрешения
class UpscaleImageCommand : public QUndoCommand {
public:
    UpscaleImageCommand(ImagoImageModel *model, int index, const QPixmap &oldPixmap, const QRectF &oldCrop, const QPixmap &newPixmap, const QRectF &newCrop, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ImagoImageModel *m_model;
    int m_index;
    QPixmap m_oldPixmap, m_newPixmap;
    QRectF m_oldCrop, m_newCrop;
};
