#include "SelectionController.h"
#include "ImageModel.h"

#include <QRectF>

SelectionController::SelectionController(ImageItemModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
}

bool SelectionController::hasSelection() const
{
    return !m_model->selectedIndices().isEmpty();
}

void SelectionController::selectItem(int index, bool addToSelection)
{
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
    for (int i = 0; i < m_model->count(); ++i) {
        m_model->setSelected(i, true);
    }
    emit selectionChanged();
}

void SelectionController::clearSelection()
{
    m_model->clearSelection();
    emit selectionChanged();
}

void SelectionController::selectInRect(qreal x, qreal y, qreal width, qreal height, bool addToSelection)
{
    QRectF selectionRect(x, y, width, height);
    selectionRect = selectionRect.normalized();
    
    if (!addToSelection) {
        m_model->clearSelection();
    }
    
    for (int i = 0; i < m_model->count(); ++i) {
        ImageData item = m_model->getItem(i);
        QRectF itemRect(item.x, item.y, item.width, item.height);
        
        if (selectionRect.intersects(itemRect)) {
            m_model->setSelected(i, true);
        }
    }
    
    emit selectionChanged();
}

int SelectionController::hitTest(qreal x, qreal y) const
{
    for (int i = m_model->count() - 1; i >= 0; --i) {
        ImageData item = m_model->getItem(i);
        QRectF rect(item.x, item.y, item.width, item.height);
        if (rect.contains(x, y)) {
            return i;
        }
    }
    return -1;
}

qreal SelectionController::getItemX(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).x;
    return 0;
}

qreal SelectionController::getItemY(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).y;
    return 0;
}

qreal SelectionController::getItemWidth(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).width;
    return 0;
}

qreal SelectionController::getItemHeight(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).height;
    return 0;
}

bool SelectionController::isItemSelected(int index) const
{
    if (index >= 0 && index < m_model->count())
        return m_model->getItem(index).selected;
    return false;
}
