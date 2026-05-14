//ClipboardController — контроллер операций с буфером обмена и добавлением изображений

#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

class ImagoImageModel;

class ClipboardController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("ClipboardController is only available via BoardController.clipboardController")

public:
    explicit ClipboardController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    //методы добавления объектов
    Q_INVOKABLE void addImage(const QUrl &imageUrl, qreal x, qreal y);
    Q_INVOKABLE void addImageFromPixmap(const QByteArray &imageData, qreal x, qreal y);
    Q_INVOKABLE void pasteFromClipboard(qreal x, qreal y);

private:
    ImagoImageModel *m_model;
    QUndoStack *m_undoStack;
};
