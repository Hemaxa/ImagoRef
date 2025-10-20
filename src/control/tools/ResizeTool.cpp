#include "ResizeTool.h"  // ✅
#include "CanvasView.h"  // ✅
#include "ImageItem.h"   // ✅
#include <QGraphicsScene>

void ResizeTool::execute()
{
    if (!m_canvasView) return;

    QList<QGraphicsItem*> selected = m_canvasView->scene()->selectedItems();

    if (selected.count() == 1) {
        ImageItem *item = dynamic_cast<ImageItem*>(selected.first());
        if (item) {
            item->setResizeMode(true);
        }
    }
}
