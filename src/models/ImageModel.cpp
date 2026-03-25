#include "ImageModel.h"
#include "CacheManager.h"
#include <QUuid>
#include <QDateTime>

ImagoImageModel::ImagoImageModel(QObject *parent) : QAbstractListModel(parent) {}

//метод получения свойтсва объекта
QVariant ImagoImageModel::data(const QModelIndex &index, int role) const
{
    //проверка существования индекса в массиве
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    //получаем нужную картинку
    const ImagoImageData &item = m_items.at(index.row());

    switch (role) {
    case IdRole: return item.id;
    case SourceRole: //источник картинки
        if (item.source.isEmpty() && !item.id.isEmpty() && !item.pixmap.isNull()) { //если картинка получена не по пути на диске (без файла)
            //возвращаем динамический URL из ImagoImageProvider
            //добавляем параметр версии, чтобы избежать кэширования старого изображения в QML
            QString urlStr = QString("image://imago/%1?v=%2").arg(item.id).arg(item.version);
            if (item.cropWidth > 0 && item.cropHeight > 0) {
                //пробрасываем параметры обрезки картинки в ImagoImageProvider
                urlStr += QString("&cx=%1&cy=%2&cw=%3&ch=%4").arg(item.cropX).arg(item.cropY).arg(item.cropWidth).arg(item.cropHeight);
            }
            return QUrl(urlStr);
        }
        return item.source;
    case XRole: return item.x;
    case YRole: return item.y;
    case WidthRole: return item.width;
    case HeightRole: return item.height;
    case RotationRole: return item.rotation;
    case ZValueRole: return item.zValue;
    case SelectedRole: return item.selected;
    case LabelRole: return item.label;
    case CropXRole: return item.cropX;
    case CropYRole: return item.cropY;
    case CropWidthRole: return item.cropWidth;
    case CropHeightRole: return item.cropHeight;
    case OpacityRole: return item.opacity;
    default: return QVariant();
    }
}

//метод установки свойтсва объекта
bool ImagoImageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //проверка существования индекса в массиве
    if (!index.isValid() || index.row() >= m_items.count())
        return false;

    ImagoImageData &item = m_items[index.row()];
    bool changed = false;

    switch (role) {
    case XRole:
        if (item.x != value.toReal()) { item.x = value.toReal(); changed = true; }
        break;
    case YRole:
        if (item.y != value.toReal()) { item.y = value.toReal(); changed = true; }
        break;
    case WidthRole:
        if (item.width != value.toReal()) { item.width = value.toReal(); changed = true; }
        break;
    case HeightRole:
        if (item.height != value.toReal()) { item.height = value.toReal(); changed = true; }
        break;
    case RotationRole:
        if (item.rotation != value.toReal()) { item.rotation = value.toReal(); changed = true; }
        break;
    case ZValueRole:
        if (item.zValue != value.toReal()) { item.zValue = value.toReal(); changed = true; }
        break;
    case SelectedRole:
        if (item.selected != value.toBool()) { item.selected = value.toBool(); changed = true; }
        break;
    case LabelRole:
        if (item.label != value.toString()) { item.label = value.toString(); changed = true; }
        break;
    case CropXRole:
        if (item.cropX != value.toReal()) { item.cropX = value.toReal(); changed = true; }
        break;
    case CropYRole:
        if (item.cropY != value.toReal()) { item.cropY = value.toReal(); changed = true; }
        break;
    case CropWidthRole:
        if (item.cropWidth != value.toReal()) { item.cropWidth = value.toReal(); changed = true; }
        break;
    case CropHeightRole:
        if (item.cropHeight != value.toReal()) { item.cropHeight = value.toReal(); changed = true; }
        break;
    case OpacityRole:
        if (item.opacity != value.toReal()) { item.opacity = value.toReal(); changed = true; }
        break;
    default:
        return false;
    }

    //сигнал о том, что данные изменились
    if (changed) {
        emit dataChanged(index, index, {role});
    }
    return changed;
}

QHash<int, QByteArray> ImagoImageModel::roleNames() const
{
    //связывание констант C++ со строковыми именами
    return {
        {IdRole, "itemId"},
        {SourceRole, "source"},
        {XRole, "modelX"},
        {YRole, "modelY"},
        {WidthRole, "modelWidth"},
        {HeightRole, "modelHeight"},
        {RotationRole, "modelRotation"},
        {ZValueRole, "zValue"},
        {SelectedRole, "modelSelected"},
        {LabelRole, "modelLabel"},
        {CropXRole, "modelCropX"},
        {CropYRole, "modelCropY"},
        {CropWidthRole, "modelCropWidth"},
        {CropHeightRole, "modelCropHeight"},
        {OpacityRole, "modelOpacity"}
    };
}

//метод разрешения на редактирование
Qt::ItemFlags ImagoImageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int ImagoImageModel::getCount() const
{
    return m_items.count();
}

int ImagoImageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.count();
}

QString ImagoImageModel::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

//добавление объекта
void ImagoImageModel::addImage(const ImagoImageData &data)
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    ImagoImageData newItem = data;
    if (newItem.id.isEmpty()) {
        //создание ID, если новый объект
        newItem.id = generateId();
    }
    m_items.append(newItem);
    endInsertRows();
    emit countChanged(); //сигнал о том, что количество объектов изменилось
}

//удаление объекта
void ImagoImageModel::removeImage(int index)
{
    if (index < 0 || index >= m_items.count())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
    emit countChanged();
}

//удаление объекта по ID
void ImagoImageModel::removeImageById(const QString &id)
{
    int idx = getIndexById(id);
    if (idx >= 0) {
        removeImage(idx);
    }
}

//полное очистка всех объектов
void ImagoImageModel::clear()
{
    if (m_items.isEmpty())
        return;

    beginResetModel();
    m_items.clear();
    endResetModel();
    emit countChanged();
}

//получение копии объекта
ImagoImageData ImagoImageModel::getItem(int index) const
{
    if (index < 0 || index >= m_items.count())
        return ImagoImageData();
    return m_items.at(index);
}

int ImagoImageModel::getIndexById(const QString &id) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QVector<ImagoImageData> ImagoImageModel::getAllItems() const
{
    return m_items;
}

void ImagoImageModel::setAllItems(const QVector<ImagoImageData> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
    emit countChanged();
}

//методы изменения параметров объекта
void ImagoImageModel::setPosition(int index, qreal x, qreal y)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].x = x;
    m_items[index].y = y;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {XRole, YRole});
}

void ImagoImageModel::setSize(int index, qreal width, qreal height)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].width = width;
    m_items[index].height = height;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {WidthRole, HeightRole});
}

void ImagoImageModel::setRotation(int index, qreal rotation)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].rotation = rotation;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {RotationRole});
}

void ImagoImageModel::setSelected(int index, bool selected)
{
    if (index < 0 || index >= m_items.count())
        return;

    if (m_items[index].selected != selected) {
        m_items[index].selected = selected;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {SelectedRole});
    }
}

void ImagoImageModel::clearSelection()
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items[i].selected) {
            m_items[i].selected = false;
            QModelIndex modelIndex = createIndex(i, 0);
            emit dataChanged(modelIndex, modelIndex, {SelectedRole});
        }
    }
}

void ImagoImageModel::setCrop(int index, qreal x, qreal y, qreal width, qreal height)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].cropX = x;
    m_items[index].cropY = y;
    m_items[index].cropWidth = width;
    m_items[index].cropHeight = height;
    
    QModelIndex modelIndex = createIndex(index, 0);
    QList<int> roles = {CropXRole, CropYRole, CropWidthRole, CropHeightRole};
    if (m_items[index].source.isEmpty()) {
        roles.append(SourceRole);
    }
    emit dataChanged(modelIndex, modelIndex, roles);
}

void ImagoImageModel::setLabel(int index, const QString &label)
{
    if (index < 0 || index >= m_items.count())
        return;

    if (m_items[index].label != label) {
        m_items[index].label = label;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {LabelRole});
    }
}

void ImagoImageModel::setOpacity(int index, qreal opacity)
{
    if (index < 0 || index >= m_items.count())
        return;

    if (m_items[index].opacity != opacity) {
        m_items[index].opacity = opacity;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {OpacityRole});
    }
}

QVariantList ImagoImageModel::getSelectedIndices() const
{
    QVariantList result;
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items[i].selected) {
            result.append(i);
        }
    }
    return result;
}

void ImagoImageModel::setPixmap(int index, const QPixmap &pixmap)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].pixmap = pixmap;
    m_items[index].source = QUrl(); //очищаем локальный путь, чтобы QML брал пиксели из ImagoImageProvider
    m_items[index].version = QDateTime::currentMSecsSinceEpoch(); //обновляем версию для сброса кэша QML
    
    //pixmap не привязан к роли в QML, но можно оповестить об изменении source
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {SourceRole});
}

void ImagoImageModel::loadPixmapFromCache(int index)
{
    if (index < 0 || index >= m_items.count())
        return;

    const ImagoImageData &item = m_items.at(index);
    if (!item.imageHash.isEmpty()) {
        QPixmap cached = CacheManager::instance().loadFromCache(item.imageHash);
        if (!cached.isNull()) {
            setPixmap(index, cached);
        }
    }
}