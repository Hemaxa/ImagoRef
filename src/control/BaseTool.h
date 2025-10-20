#pragma once

#include <QObject>

class CanvasView;
class QUndoStack;

class BaseTool : public QObject
{
    Q_OBJECT

public:
    explicit BaseTool(QObject* parent = nullptr);
    virtual ~BaseTool() = default;

    virtual void setContext(CanvasView* view, QUndoStack* stack);

protected:
    CanvasView* m_canvasView;
    QUndoStack* m_undoStack;
};
