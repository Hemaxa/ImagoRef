#include "ImageModel.h"
#include <QUuid>

ImageItemModel::ImageItemModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ImageItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.count();
}

QVariant ImageItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.count())
        return QVariant();

    const ImageData &item = m_items.at(index.row());

    switch (role) {
    case IdRole: return item.id;
    case SourceRole: return item.source;
    case XRole: return item.x;
    case YRole: return item.y;
    case WidthRole: return item.width;
    case HeightRole: return item.height;
    case RotationRole: return item.rotation;
    case ZValueRole: return item.zValue;
    case SelectedRole: return item.selected;
    default: return QVariant();
    }
}

bool ImageItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_items.count())
        return false;

    ImageData &item = m_items[index.row()];
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
    default:
        return false;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
    }
    return changed;
}

QHash<int, QByteArray> ImageItemModel::roleNames() const
{
    return {
        {IdRole, "itemId"},
        {SourceRole, "source"},
        {XRole, "modelX"},
        {YRole, "modelY"},
        {WidthRole, "modelWidth"},
        {HeightRole, "modelHeight"},
        {RotationRole, "modelRotation"},
        {ZValueRole, "zValue"},
        {SelectedRole, "selected"}
    };
}

Qt::ItemFlags ImageItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int ImageItemModel::count() const
{
    return m_items.count();
}

QString ImageItemModel::generateId()
{
    return QString("img_%1").arg(++m_idCounter);
}

void ImageItemModel::addImage(const ImageData &data)
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    ImageData newItem = data;
    if (newItem.id.isEmpty()) {
        newItem.id = generateId();
    }
    m_items.append(newItem);
    endInsertRows();
    emit countChanged();
}

void ImageItemModel::removeImage(int index)
{
    if (index < 0 || index >= m_items.count())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
    emit countChanged();
}

void ImageItemModel::removeById(const QString &id)
{
    int idx = indexById(id);
    if (idx >= 0) {
        removeImage(idx);
    }
}

void ImageItemModel::clear()
{
    if (m_items.isEmpty())
        return;

    beginResetModel();
    m_items.clear();
    endResetModel();
    emit countChanged();
}

ImageData ImageItemModel::getItem(int index) const
{
    if (index < 0 || index >= m_items.count())
        return ImageData();
    return m_items.at(index);
}

int ImageItemModel::indexById(const QString &id) const
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QVector<ImageData> ImageItemModel::allItems() const
{
    return m_items;
}

void ImageItemModel::setItems(const QVector<ImageData> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
    emit countChanged();
}

void ImageItemModel::updatePosition(int index, qreal x, qreal y)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].x = x;
    m_items[index].y = y;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {XRole, YRole});
}

void ImageItemModel::updateSize(int index, qreal width, qreal height)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].width = width;
    m_items[index].height = height;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {WidthRole, HeightRole});
}

void ImageItemModel::updateRotation(int index, qreal rotation)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].rotation = rotation;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {RotationRole});
}

void ImageItemModel::setSelected(int index, bool selected)
{
    if (index < 0 || index >= m_items.count())
        return;

    if (m_items[index].selected != selected) {
        m_items[index].selected = selected;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {SelectedRole});
    }
}

void ImageItemModel::clearSelection()
{
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items[i].selected) {
            m_items[i].selected = false;
            QModelIndex modelIndex = createIndex(i, 0);
            emit dataChanged(modelIndex, modelIndex, {SelectedRole});
        }
    }
}

QVariantList ImageItemModel::selectedIndices() const
{
    QVariantList result;
    for (int i = 0; i < m_items.count(); ++i) {
        if (m_items[i].selected) {
            result.append(i);
        }
    }
    return result;
}

void ImageItemModel::updatePixmap(int index, const QPixmap &pixmap)
{
    if (index < 0 || index >= m_items.count())
        return;

    m_items[index].pixmap = pixmap;
    // Pixmap не привязан к роли в QML, но можно оповестить об изменении source
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {SourceRole});
}
