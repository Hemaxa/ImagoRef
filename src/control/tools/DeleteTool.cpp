#include "DeleteTool.h"    // ✅
#include "CanvasView.h"    // ✅
#include "UndoRedoTool.h"  // ✅

#include <QGraphicsScene>
#include <QUndoStack>

void DeleteTool::execute()
{
    if (!m_canvasView || !m_undoStack) return;

    QList<QGraphicsItem*> selected = m_canvasView->scene()->selectedItems();
    if (selected.isEmpty()) return;

    m_undoStack->push(new RemoveCommand(selected, m_canvasView->scene()));
}
