#include "ClipboardController.h"
#include "ImageModel.h"
#include "StackController.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QBuffer>
#include <QFileInfo>
#include <QImageReader>

ClipboardController::ClipboardController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
{
}

void ClipboardController::addImage(const QUrl &imageUrl, qreal x, qreal y)
{
    QString filePath = imageUrl.toLocalFile();
    QPixmap pixmap(filePath);
    
    if (pixmap.isNull()) {
        return;
    }

    ImageData data;
    data.source = imageUrl;
    data.pixmap = pixmap;
    data.x = x;
    data.y = y;
    data.width = pixmap.width();
    data.height = pixmap.height();
    
    m_model->addImage(data);
    
    m_undoStack->push(new AddImageCommand(
        m_model, 
        m_model->getItem(m_model->getCount() - 1).id,
        imageUrl, x, y, data.width, data.height
    ));
}

void ClipboardController::addImageFromPixmap(const QByteArray &imageData, qreal x, qreal y)
{
    QImage image;
    if (!image.loadFromData(imageData)) {
        return;
    }
    
    QPixmap pixmap = QPixmap::fromImage(image);
    
    ImageData data;
    data.pixmap = pixmap;
    data.x = x;
    data.y = y;
    data.width = pixmap.width();
    data.height = pixmap.height();
    
    m_model->addImage(data);
    
    m_undoStack->push(new AddImageCommand(
        m_model,
        m_model->getItem(m_model->getCount() - 1).id,
        QUrl(), x, y, data.width, data.height
    ));
}

void ClipboardController::pasteFromClipboard(qreal x, qreal y)
{
    const QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    
    static const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

    if (mimeData->hasUrls()) {
        for (const QUrl &url : mimeData->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QString extension = QFileInfo(filePath).suffix().toLower();
                if (supportedFormats.contains(extension.toUtf8())) {
                    addImage(url, x, y);
                    return;
                }
            }
        }
    }

    const QImage image = clipboard->image();
    if (!image.isNull()) {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        addImageFromPixmap(data, x, y);
    }
}
