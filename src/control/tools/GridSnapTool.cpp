#include "GridSnapTool.h"
#include "CanvasView.h"
#include "ImageItem.h"
#include "StackManager.h"

#include <QGraphicsScene>
#include <QUndoStack>

void GridSnapTool::execute()
{
    if (!m_canvasView || !m_undoStack) return;

    const int gridSize = m_canvasView->getGridSize();
    if (gridSize <= 0) return;

    m_undoStack->beginMacro("Привязка к сетке");

    for (QGraphicsItem *item : m_canvasView->scene()->items()) {
        if (ImageItem* imgItem = dynamic_cast<ImageItem*>(item)) {
            QPointF currentPos = imgItem->pos();
            qreal newX = round(currentPos.x() / gridSize) * gridSize;
            qreal newY = round(currentPos.y() / gridSize) * gridSize;

            if (currentPos != QPointF(newX, newY)) {
                imgItem->setPos(newX, newY);
                m_undoStack->push(new MoveCommand(imgItem, currentPos));
            }
        }
    }

    m_undoStack->endMacro();
}
