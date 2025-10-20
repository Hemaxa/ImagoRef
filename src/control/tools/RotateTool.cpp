#include "RotateTool.h"    // ✅
#include "CanvasView.h"    // ✅
#include "ImageItem.h"     // ✅
#include "UndoRedoTool.h"  // ✅
#include <QGraphicsScene>
#include <QUndoStack>

void RotateTool::rotateLeft()
{
    if (!m_canvasView || !m_undoStack) return;

    QList<QGraphicsItem*> selected = m_canvasView->scene()->selectedItems();
    if (selected.isEmpty()) return;

    m_undoStack->beginMacro("Вращение против часовой");
    for (QGraphicsItem *item : selected) {
        if (ImageItem *imgItem = dynamic_cast<ImageItem*>(item)) {
            m_undoStack->push(new RotateCommand(imgItem, -90.0));
        }
    }
    m_undoStack->endMacro();
}

void RotateTool::rotateRight()
{
    if (!m_canvasView || !m_undoStack) return;

    QList<QGraphicsItem*> selected = m_canvasView->scene()->selectedItems();
    if (selected.isEmpty()) return;

    m_undoStack->beginMacro("Вращение по часовой");
    for (QGraphicsItem *item : selected) {
        if (ImageItem *imgItem = dynamic_cast<ImageItem*>(item)) {
            m_undoStack->push(new RotateCommand(imgItem, 90.0));
        }
    }
    m_undoStack->endMacro();
}
