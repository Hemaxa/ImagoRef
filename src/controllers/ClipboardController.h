#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

class ImageItemModel;

/**
 * @brief ClipboardController — контроллер операций с буфером обмена и добавлением изображений.
 */
class ClipboardController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("ClipboardController is only available via BoardController.clipboardController")

public:
    explicit ClipboardController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    Q_INVOKABLE void addImage(const QUrl &imageUrl, qreal x, qreal y);
    Q_INVOKABLE void addImageFromPixmap(const QByteArray &imageData, qreal x, qreal y);
    Q_INVOKABLE void pasteFromClipboard(qreal x, qreal y);

private:
    ImageItemModel *m_model;
    QUndoStack *m_undoStack;
};
