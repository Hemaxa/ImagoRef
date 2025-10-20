#include "BaseTool.h" // ✅

BaseTool::BaseTool(QObject* parent)
    : QObject(parent), m_canvasView(nullptr), m_undoStack(nullptr)
{
}

void BaseTool::setContext(CanvasView* view, QUndoStack* stack)
{
    m_canvasView = view;
    m_undoStack = stack;
}
