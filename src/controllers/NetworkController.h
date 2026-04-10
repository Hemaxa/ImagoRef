#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QTimer>
#include <QtQml/qqml.h>

class StorageController;

class NetworkController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("NetworkController can only be accessed via BoardController")
    
public:
    explicit NetworkController(StorageController *storage, QObject *parent = nullptr);
    ~NetworkController();

    Q_INVOKABLE void connectToBoard(const QString &boardId);
    Q_INVOKABLE void disconnectFromBoard();

signals:
    void itemUpdatedFromNetwork(const QString &itemId);
    
private slots:
    void processSyncQueue();
    // WebSocket handlers
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);

private:
    void handleUploadTask(int taskId, const QJsonObject &payload);
    void handleWebSocketTask(int taskId, const QString &action, const QJsonObject &payload);
    
    // API logic to fetch presigned URL and upload -> on finish delete task & send WS
    void uploadImageToS3(int taskId, const QString &itemId, const QString &imageHash);

    void fetchMetadataAndMissingImages();
    void downloadImageFromS3(const QString &hash, const QString &url);

    StorageController *m_storageController;
    QNetworkAccessManager *m_networkManager;
    QWebSocket *m_webSocket;
    QTimer *m_syncTimer;
    QString m_currentBoardId;

    bool m_isUploading = false;
    
    // API routes (Mock)
    const QString API_BASE_URL = "https://imagoref.ru/api";
    const QString WS_URL = "wss://imagoref.ru/ws";
};
