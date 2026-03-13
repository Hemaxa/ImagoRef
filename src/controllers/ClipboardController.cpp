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

ClipboardController::ClipboardController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
{}

//метод для добавления картинки с жесткого диска (по URL/пути)
void ClipboardController::addImage(const QUrl &imageUrl, qreal x, qreal y)
{
    //получаем путь в ОС и загружаем картинку оперативную память
    QString filePath = imageUrl.toLocalFile();
    QPixmap pixmap(filePath);
    
    if (pixmap.isNull()) {
        return;
    }

    //собираем структуру с данными новой картинки
    ImagoImageData data;
    data.source = imageUrl;
    data.pixmap = pixmap;
    data.x = x;
    data.y = y;
    data.width = pixmap.width();
    data.height = pixmap.height();
    
    //добавляем картинку в Модель
    m_model->addImage(data);
    
    //кладем в историю команду добавления
    m_undoStack->push(new AddImageCommand(
        m_model, 
        m_model->getItem(m_model->getCount() - 1).id,
        imageUrl, x, y, data.width, data.height
    ));
}

//метод для добавления картинки из сырых байтов (например, из буфера обмена)
void ClipboardController::addImageFromPixmap(const QByteArray &imageData, qreal x, qreal y)
{
    QImage image;
    //пытаемся превратить сырые байты в QImage
    if (!image.loadFromData(imageData)) {
        return;
    }
    
    QPixmap pixmap = QPixmap::fromImage(image);
    
    ImagoImageData data;
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

//главный метод вставки
void ClipboardController::pasteFromClipboard(qreal x, qreal y)
{
    //получаем доступ к системному буферу обмена ОС (Windows/macOS/Linux)
    const QClipboard *clipboard = QGuiApplication::clipboard();

    //получаем сырые данные из буфера (это может быть текст, файл, картинка или всё сразу)
    const QMimeData* mimeData = clipboard->mimeData();
    
    //спрашиваем у Qt, какие форматы файлов (png, jpg, webp) поддерживает система
    static const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

    //если ользователь скопировал файл в проводнике
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

    //если пользователь скопировал саму картинку
    const QImage image = clipboard->image();
    if (!image.isNull()) {
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        addImageFromPixmap(data, x, y);
    }
}
