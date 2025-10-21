#pragma once

#include "BaseTool.h"

class GridSnapTool : public BaseTool
{
    Q_OBJECT

public:
    explicit GridSnapTool(QObject* parent = nullptr) : BaseTool(parent) {}

public slots:
    void execute();
};
