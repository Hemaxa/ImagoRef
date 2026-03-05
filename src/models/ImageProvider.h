#pragma once

#include <QQuickImageProvider>
#include <QPixmap>

class ImageItemModel;

/**
 * @brief ImagoImageProvider — провайдер изображений для QML.
 * Позволяет QML загружать pixmap по URL вида image://imago/<imageId>
 * Это необходимо для отображения изображений, загруженных из файла .iref
 * или вставленных из буфера обмена (у которых нет файлового пути).
 */
class ImagoImageProvider : public QQuickImageProvider {
public:
    explicit ImagoImageProvider(ImageItemModel *model);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    ImageItemModel *m_model;
};
