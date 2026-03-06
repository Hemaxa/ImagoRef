#include "ImageProvider.h"
#include "ImageModel.h"

ImagoImageProvider* ImagoImageProvider::s_instance = nullptr;

ImagoImageProvider::ImagoImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    s_instance = this;
}

ImagoImageProvider* ImagoImageProvider::instance()
{
    return s_instance;
}

void ImagoImageProvider::setModel(ImageItemModel *model)
{
    m_model = model;
}

QPixmap ImagoImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)

    if (!m_model) {
        if (size) *size = QSize(0, 0);
        return QPixmap();
    }

    // id приходит в виде "<imageId>?t=<timestamp>" — отсекаем параметры
    QString imageId = id.section('?', 0, 0);

    int index = m_model->indexById(imageId);
    if (index < 0) {
        // Элемент не найден — возвращаем пустой pixmap
        if (size) *size = QSize(0, 0);
        return QPixmap();
    }

    ImageData item = m_model->getItem(index);
    QPixmap pixmap = item.pixmap;

    if (pixmap.isNull()) {
        if (size) *size = QSize(0, 0);
        return QPixmap();
    }

    if (size) *size = pixmap.size();
    return pixmap;
}
