#pragma once

#include "BaseTool.h"

class ResizeTool : public BaseTool
{
    Q_OBJECT

public:
    explicit ResizeTool(QObject* parent = nullptr) : BaseTool(parent) {}

public slots:
    void execute();
};
