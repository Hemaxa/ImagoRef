//ImageModel - модель для хранения списка изображений на холсте

#pragma once

#include <QAbstractListModel> //списки в Qt
#include <QPixmap> //изображения в Qt
#include <QUrl> //класс для работы с путями
#include <QtQml/qqml.h>

//ImageData - структура данных для хранения информации об изображении
struct ImageData {
    QString id; //уникальный идентификатор
    QUrl source; //путь к изображению
    QPixmap pixmap; //оригинальный пиксмап
    qreal x = 0; //координата x
    qreal y = 0; //координата y
    qreal width = 0; //ширина
    qreal height = 0; //высота
    qreal rotation = 0; //угол
    qreal zValue = 0; //уровень
    bool selected = false; //флаг выбора
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
        SelectedRole               // Статус выделения (bool)
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
    ImageData getItem(int index) const;
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
    
    // Обновление пиксмапа (для обрезки)
    void updatePixmap(int index, const QPixmap &pixmap);

signals:
    void countChanged();

private:
    QVector<ImageData> m_items;
    int m_idCounter = 0;
    
    QString generateId();
};
