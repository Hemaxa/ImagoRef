//ImagoImageProvider — провайдер изображений для QML. Позволяет QML загружать pixmap по URL вида image://imago/<imageId>
//это необходимо для отображения изображений, загруженных из файла .iref или вставленных из буфера обмена (у которых нет файлового пути)

#pragma once

#include <QQuickImageProvider>
#include <QPixmap>

class ImagoImageModel;

class ImagoImageProvider : public QQuickImageProvider {
public:
    explicit ImagoImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    void registerModel(ImagoImageModel *model);
    void unregisterModel(ImagoImageModel *model);
    static ImagoImageProvider* instance();

private:
    QList<ImagoImageModel*> m_models;
    static ImagoImageProvider* s_instance;
};
