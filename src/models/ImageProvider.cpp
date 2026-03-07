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

void ImagoImageProvider::registerModel(ImageItemModel *model)
{
    if (model && !m_models.contains(model)) {
        m_models.append(model);
    }
}

void ImagoImageProvider::unregisterModel(ImageItemModel *model)
{
    m_models.removeAll(model);
}

QPixmap ImagoImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize)

    // id приходит в виде "<imageId>?t=<timestamp>" — отсекаем параметры
    QString imageId = id.section('?', 0, 0);

    for (ImageItemModel *model : std::as_const(m_models)) {
        int index = model->indexById(imageId);
        if (index >= 0) {
            ImageData item = model->getItem(index);
            QPixmap pixmap = item.pixmap;
            
            if (!pixmap.isNull()) {
                if (size) *size = pixmap.size();
                return pixmap;
            }
        }
    }

    if (size) *size = QSize(0, 0);
    return QPixmap();
}
