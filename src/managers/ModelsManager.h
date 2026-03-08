#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QQmlEngine>
#include <QJSEngine>

class ModelsManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isModelDownloaded READ isModelDownloaded NOTIFY modelDownloadedChanged)
    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY downloadingChanged)
    Q_PROPERTY(qreal downloadProgress READ downloadProgress NOTIFY progressChanged)

public:
    static ModelsManager& instance();
    static ModelsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

    bool isModelDownloaded() const;
    bool isDownloading() const;
    qreal downloadProgress() const;

    Q_INVOKABLE void downloadModel();
    Q_INVOKABLE void deleteModel();
    
    QString getModelPath() const;
    QString getParamPath() const;

signals:
    void modelDownloadedChanged();
    void downloadingChanged();
    void progressChanged();
    void downloadFinished(bool success, const QString &errorMsg);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();

private:
    explicit ModelsManager(QObject *parent = nullptr);
    ModelsManager(const ModelsManager&) = delete;
    ModelsManager& operator=(const ModelsManager&) = delete;

    void extractArchive(const QString &zipPath);
    void checkModelExists();

    bool m_isModelDownloaded = false;
    bool m_isDownloading = false;
    qreal m_downloadProgress = 0.0;
    
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_networkReply;
    QString m_modelsDir;
};
