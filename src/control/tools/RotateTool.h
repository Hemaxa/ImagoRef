#pragma once
#include "BaseTool.h" // ✅

class RotateTool : public BaseTool
{
    Q_OBJECT
public:
    explicit RotateTool(QObject* parent = nullptr) : BaseTool(parent) {}
public slots:
    void rotateLeft();
    void rotateRight();
};
