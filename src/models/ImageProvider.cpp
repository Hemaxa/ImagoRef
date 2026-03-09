#include "ImageProvider.h"
#include "ImageModel.h"

#include <QUrlQuery>

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

    // id приходит в виде "<imageId>?v=<timestamp>&cx=..." — отсекаем параметры
    QString imageId = id.section('?', 0, 0);
    QUrlQuery query(id.section('?', 1));

    for (ImageItemModel *model : std::as_const(m_models)) {
        int index = model->indexById(imageId);
        if (index >= 0) {
            ImageData item = model->getItem(index);
            QPixmap pixmap = item.pixmap;
            
            if (!pixmap.isNull()) {
                if (query.hasQueryItem("cw") && query.hasQueryItem("ch")) {
                    qreal cx = query.queryItemValue("cx").toDouble();
                    qreal cy = query.queryItemValue("cy").toDouble();
                    qreal cw = query.queryItemValue("cw").toDouble();
                    qreal ch = query.queryItemValue("ch").toDouble();
                    if (cw > 0 && ch > 0) {
                        pixmap = pixmap.copy(cx, cy, cw, ch);
                    }
                }

                if (size) *size = pixmap.size();
                return pixmap;
            }
        }
    }

    if (size) *size = QSize(0, 0);
    return QPixmap();
}
