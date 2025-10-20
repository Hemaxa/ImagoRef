#pragma once
#include "BaseTool.h" // ✅

class ZoomTool : public BaseTool
{
    Q_OBJECT
public:
    explicit ZoomTool(QObject* parent = nullptr) : BaseTool(parent) {}
public slots:
    void zoomIn();
    void zoomOut();
};
