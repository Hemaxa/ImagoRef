//ImageModel - модель для хранения списка изображений на холсте

#pragma once

#include <QAbstractListModel> //умные списки в Qt, через которые QML автоматически перерисует элемент при изменении свойств
#include <QPixmap> //изображения в Qt
#include <QUrl> //класс для работы с путями
#include <QtQml/qqml.h>

//ImageData - структура данных для хранения информации об одном изображении
struct ImageData {
    QString id; //уникальный идентификатор изображения
    QUrl source; //путь к изображению
    QPixmap pixmap; //оригинальное изображение (QPixmap)
    qreal x = 0; //координата x изображения
    qreal y = 0; //координата y изображения
    qreal width = 0; //ширина изображения
    qreal height = 0; //высота изображения
    qreal rotation = 0; //угол поворота изображения
    qreal zValue = 0; //уровень изображения по высоте
    bool selected = false; //флаг выбора изображения
    QString label; //подпись изображения
    
    //параметры обрезки (в координатах исходного изображения)
    qreal cropX = 0;
    qreal cropY = 0;
    qreal cropWidth = 0;  // 0 означает "нет обрезки" (полная ширина)
    qreal cropHeight = 0; // 0 означает "нет обрезки" (полная высота)
};

class ImageItemModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT //позволяет создавать класс из QML

    //свойства, доступные в QML
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1, // Уникальный ID (строка)
        SourceRole,                // URL источника изображение
        XRole,                     // Позиция X
        YRole,                     // Позиция Y
        WidthRole,                 // Ширина
        HeightRole,                // Высота
        RotationRole,              // Угол поворота (градусы)
        ZValueRole,                // Z-index (слой)
        SelectedRole,              // Статус выделения (bool)
        LabelRole,                 // Подпись изображения (строка)
        CropXRole,                 // Обрезка X (исходные координаты)
        CropYRole,                 // Обрезка Y (исходные координаты)
        CropWidthRole,             // Обрезка Width (исходные координаты)
        CropHeightRole             // Обрезка Height (исходные координаты)
    };

    explicit ImageItemModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Публичные методы для управления
    int count() const;
    void addImage(const ImageData &data);
    void removeImage(int index);
    void removeById(const QString &id);
    void clear();
    Q_INVOKABLE ImageData getItem(int index) const;
    int indexById(const QString &id) const;
    
    // Для сериализации
    QVector<ImageData> allItems() const;
    void setItems(const QVector<ImageData> &items);

    // Методы для доступа из QML
    Q_INVOKABLE void updatePosition(int index, qreal x, qreal y);
    Q_INVOKABLE void updateSize(int index, qreal width, qreal height);
    Q_INVOKABLE void updateRotation(int index, qreal rotation);
    Q_INVOKABLE void setSelected(int index, bool selected);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE QVariantList selectedIndices() const;
    
    // Обновление обрезки (non-destructive)
    Q_INVOKABLE void updateCrop(int index, qreal x, qreal y, qreal width, qreal height);
    
    // Обновление подписи
    Q_INVOKABLE void updateLabel(int index, const QString &label);
    
    // Обновление пиксмапа (для обрезки)
    void updatePixmap(int index, const QPixmap &pixmap);

signals:
    void countChanged();

private:
    QVector<ImageData> m_items;
    int m_idCounter = 0;
    
    QString generateId();
};
