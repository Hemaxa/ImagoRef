//ImageModel - модель для хранения списка изображений на холсте

#pragma once //защита от двойного включения файла при компиляции

#include <QAbstractListModel> //умные списки в Qt, через которые QML автоматически перерисует элемент при изменении свойств
#include <QPixmap> //изображения в Qt
#include <QUrl> //класс для работы с путями
#include <QtQml/qqml.h> //работа с QML

//ImagoImageData - структура данных для хранения информации об одном изображении
struct ImagoImageData {
    //данные изображения
    QString id; //уникальный идентификатор изображения
    QUrl source; //путь к изображению
    QPixmap pixmap; //оригинальное изображение (QPixmap)
    
    //параметры изображения
    qreal x = 0; //координата x изображения
    qreal y = 0; //координата y изображения
    qreal width = 0; //ширина изображения
    qreal height = 0; //высота изображения
    qreal rotation = 0; //угол поворота изображения
    qreal zValue = 0; //уровень изображения по высоте
    bool selected = false; //флаг выбора изображения
    QString label; //подпись изображения
    QString imageHash; //хэш изображения
    
    //параметры обрезки (в координатах исходного изображения)
    qreal cropX = 0; //координата x обрезанной части
    qreal cropY = 0; //координата у обрезанной части
    qreal cropWidth = 0; //ширина обрезки
    qreal cropHeight = 0; //высота обрезки
    
    qreal opacity = 1.0; //непрозрачность изображения (от 0.0 до 1.0)
    
    qint64 version = 0; //версия изображения для обновления кэша QML
};

class ImagoImageModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT //позволяет создавать класс из QML

    //свойства, доступные в QML
    Q_PROPERTY(int count READ getCount NOTIFY countChanged)

public:
    //словарь переменных для C++ и QML
    enum Roles {
        IdRole = Qt::UserRole + 1,
        SourceRole,
        XRole,
        YRole,
        WidthRole,
        HeightRole,
        RotationRole,
        ZValueRole,
        SelectedRole,
        LabelRole,
        CropXRole,
        CropYRole,
        CropWidthRole,
        CropHeightRole,
        OpacityRole
    };

    //конструктор
    explicit ImagoImageModel(QObject *parent = nullptr);

    //переопределнные методы для QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override; //количество объектов
    QVariant data(const QModelIndex &index, int role) const override; //получение свойств
    bool setData(const QModelIndex &index, const QVariant &value, int role) override; //установка свойств
    QHash<int, QByteArray> roleNames() const override; //связь QML и C++
    Qt::ItemFlags flags(const QModelIndex &index) const override; //разрешения на редактирования объекта

    //методы для управления
    int getCount() const;
    void addImage(const ImagoImageData &data);
    void removeImage(int index);
    void removeImageById(const QString &id);
    void clear();
    Q_INVOKABLE ImagoImageData getItem(int index) const;
    int getIndexById(const QString &id) const;
    
    //работа со всеми объектами сразу (FileController)
    QVector<ImagoImageData> getAllItems() const;
    void setAllItems(const QVector<ImagoImageData> &items);

    //методы для доступа из QML
    Q_INVOKABLE void setPosition(int index, qreal x, qreal y);
    Q_INVOKABLE void setSize(int index, qreal width, qreal height);
    Q_INVOKABLE void setRotation(int index, qreal rotation);
    Q_INVOKABLE void setCrop(int index, qreal x, qreal y, qreal width, qreal height);
    Q_INVOKABLE void setLabel(int index, const QString &label);
    Q_INVOKABLE void setSelected(int index, bool selected);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE QVariantList getSelectedIndices() const;
    Q_INVOKABLE void setOpacity(int index, qreal opacity);
    Q_INVOKABLE void loadPixmapFromCache(int index);

    void setPixmap(int index, const QPixmap &pixmap);

signals:
    //изменение количества объектов
    void countChanged();

private:
    QVector<ImagoImageData> m_items; //вектор всех объектов программы 
    
    QString generateId(); //генерация уникального ID объекта
};
