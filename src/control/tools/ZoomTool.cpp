#include "ZoomTool.h"
#include "CanvasView.h"

void ZoomTool::zoomIn()
{
    if (m_canvasView) m_canvasView->scale(1.15, 1.15);
}

void ZoomTool::zoomOut()
{
    if (m_canvasView) m_canvasView->scale(1.0 / 1.15, 1.0 / 1.15);
}
