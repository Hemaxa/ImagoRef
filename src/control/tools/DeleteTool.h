#pragma once

#include "BaseTool.h"

class DeleteTool : public BaseTool
{
    Q_OBJECT

public:
    explicit DeleteTool(QObject* parent = nullptr) : BaseTool(parent) {}

public slots:
    void execute();
};
