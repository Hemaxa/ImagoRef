#include "SelectionController.h"
#include "ImageModel.h"

#include <QRectF>
#include <QTransform>
#include <QPolygonF>

SelectionController::SelectionController(ImageItemModel *model, QObject *parent) : QObject(parent), m_model(model) {}

bool SelectionController::getHasSelection() const
{
    return !m_model->getSelectedIndices().isEmpty();
}

void SelectionController::selectItem(int index, bool addToSelection)
{
    //если клик без зажатого Shift/Ctrl, сначала снимаем выделение со всех остальных
    if (!addToSelection) {
        m_model->clearSelection();
    }
    m_model->setSelected(index, true);
    emit selectionChanged();
}

void SelectionController::deselectItem(int index)
{
    m_model->setSelected(index, false);
    emit selectionChanged();
}

void SelectionController::toggleSelection(int index)
{
    ImageData item = m_model->getItem(index);
    m_model->setSelected(index, !item.selected);
    emit selectionChanged();
}

void SelectionController::selectAll()
{
    for (int i = 0; i < m_model->getCount(); ++i) {
        m_model->setSelected(i, true);
    }
    emit selectionChanged();
}

void SelectionController::clearSelection()
{
    m_model->clearSelection();
    emit selectionChanged();
}

//логика рамочного выделения
void SelectionController::selectInRect(qreal x, qreal y, qreal width, qreal height, bool addToSelection)
{
    QRectF selectionRect(x, y, width, height);
    selectionRect = selectionRect.normalized(); //normalized() исправляет отрицательные ширину/высоту, он пересчитает координаты так, чтобы размеры были положительными
    
    if (!addToSelection) {
        m_model->clearSelection();
    }
    
    QPolygonF selectionPolygon(selectionRect);
    
    //проходим по всем картинкам и проверяем, пересекается ли их прямоугольник с прямоугольником рамки
    for (int i = 0; i < m_model->getCount(); ++i) {
        ImageData item = m_model->getItem(i);
        QRectF itemRect(item.x, item.y, item.width, item.height);
        
        QTransform transform;
        transform.translate(item.x + item.width / 2.0, item.y + item.height / 2.0);
        transform.rotate(item.rotation);
        transform.translate(-(item.x + item.width / 2.0), -(item.y + item.height / 2.0));
        
        QPolygonF itemPolygonF = transform.map(itemRect);
        
        if (!selectionPolygon.intersected(itemPolygonF).isEmpty() || selectionPolygon.containsPoint(itemPolygonF.boundingRect().center(), Qt::OddEvenFill)) {
            m_model->setSelected(i, true);
        }
    }
    
    emit selectionChanged();
}

int SelectionController::hitTest(qreal x, qreal y) const
{
    for (int i = m_model->getCount() - 1; i >= 0; --i) {
        ImageData item = m_model->getItem(i);
        QRectF rect(item.x, item.y, item.width, item.height);
        
        QTransform transform;
        transform.translate(item.x + item.width / 2.0, item.y + item.height / 2.0);
        transform.rotate(item.rotation);
        transform.translate(-(item.x + item.width / 2.0), -(item.y + item.height / 2.0));
        
        QPolygonF polygon = transform.map(rect);
        if (polygon.containsPoint(QPointF(x, y), Qt::OddEvenFill)) {
            return i;
        }
    }
    return -1;
}

qreal SelectionController::getItemX(int index) const
{
    if (index >= 0 && index < m_model->getCount())
        return m_model->getItem(index).x;
    return 0;
}

qreal SelectionController::getItemY(int index) const
{
    if (index >= 0 && index < m_model->getCount())
        return m_model->getItem(index).y;
    return 0;
}

qreal SelectionController::getItemWidth(int index) const
{
    if (index >= 0 && index < m_model->getCount())
        return m_model->getItem(index).width;
    return 0;
}

qreal SelectionController::getItemHeight(int index) const
{
    if (index >= 0 && index < m_model->getCount())
        return m_model->getItem(index).height;
    return 0;
}

bool SelectionController::getIsItemSelected(int index) const
{
    if (index >= 0 && index < m_model->getCount())
        return m_model->getItem(index).selected;
    return false;
}
