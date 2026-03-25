#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>

class ImagoImageModel;
class QUndoStack;

class CloudController : public QObject {
    Q_OBJECT
public:
    explicit CloudController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    Q_INVOKABLE void syncUp(const QString &boardId);
    Q_INVOKABLE void syncDown(const QString &boardId);

signals:
    void cloudSyncStarted();
    void cloudSyncProgress(int current, int total);
    void cloudSyncFinished();

private slots:
    void onSyncHashesFinished();
    void onS3UploadFinished();
    void onMetadataUploadFinished();

    void onMetadataDownloadFinished();
    void onS3DownloadFinished();

private:
    void uploadMetadata(const QString &boardId);
    void finalizeSyncDown();

    ImagoImageModel *m_model;
    QUndoStack *m_undoStack;
    QNetworkAccessManager *m_networkManager;

    // Стейт загрузки (Upload)
    QString m_currentUploadBoardId;
    int m_uploadCount;
    int m_uploadTotal;

    // Стейт скачивания (Download)
    QString m_currentDownloadBoardId;
    int m_downloadCount;
    int m_downloadTotal;
    QJsonObject m_downloadedMetadata;

    const QString API_BASE_URL = "http://localhost:8000/api";
};
