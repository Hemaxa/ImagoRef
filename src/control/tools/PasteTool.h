#pragma once

#include "BaseTool.h"

#include <QByteArray>

class PasteTool : public BaseTool
{
    Q_OBJECT

public:
    explicit PasteTool(QObject* parent = nullptr);

public slots:
    void execute();

private:
    const QList<QByteArray> m_supportedFormats;
};
