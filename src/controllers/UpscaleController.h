#pragma once

#include <QObject>
#include <QPixmap>
#include <QRunnable>
#include <QImage>
#include <QSet>

class ImageItemModel;
class ModelsManager;

// Worker to run upscale in a background thread
class UpscaleWorker : public QObject, public QRunnable {
    Q_OBJECT
public:
    UpscaleWorker(int index, const QImage &image, const QString &modelPath, const QString &paramPath);

    void run() override;

signals:
    void finished(int index, QImage result);
    void failed(int index, QString error);

private:
    int m_index;
    QImage m_image;
    QString m_modelPath;
    QString m_paramPath;
};

// Controller to handle upscale tasks
class UpscaleController : public QObject {
    Q_OBJECT

public:
    explicit UpscaleController(ImageItemModel *model, ModelsManager *modelsManager, QObject *parent = nullptr);

    Q_INVOKABLE void upscaleImage(int index);

signals:
    void upscaleStarted(int index);
    void upscaleFinished(int index);
    void upscaleFailed(int index, QString error);

private slots:
    void onUpscaleFinished(int index, QImage result);
    void onUpscaleFailed(int index, QString error);

private:
    ImageItemModel *m_model;
    ModelsManager *m_modelsManager;
    QSet<int> m_activeTasks;
};
