#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QtQml/qqml.h>
#include <QJsonObject>
#include <QSet>

class StorageController;

class NetworkController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("NetworkController can only be accessed via BoardController")
    
public:
    explicit NetworkController(StorageController *storage, QObject *parent = nullptr);
    ~NetworkController();

    Q_INVOKABLE void connectToBoard(const QString &boardId);
    Q_INVOKABLE void disconnectFromBoard();
    
    // НОВЫЙ МЕТОД ДЛЯ Ctrl+S
    Q_INVOKABLE void syncBoardToServer();

signals:
    void itemUpdatedFromNetwork(const QString &itemId);
    
    // Сигналы для UI (можно показывать крутилку загрузки)
    void syncStarted();
    void syncFinished(bool success);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);

private:
    // Новые методы пакетной синхронизации
    void checkMissingImagesAndUpload(const QJsonObject& boardState, const QSet<QString>& hashes);
    void uploadToS3(const QString& hash, const QString& url);
    void pushStateToServer(const QJsonObject& boardState);

    void fetchMetadataAndMissingImages();
    void downloadImageFromS3(const QString &hash, const QString &url);
    void syncLocalBoardToServer();

    StorageController *m_storageController;
    QNetworkAccessManager *m_networkManager;
    QWebSocket *m_webSocket;
    QString m_currentBoardId;

    // Переменные для отслеживания пакетной загрузки
    int m_pendingUploads = 0;
    QJsonObject m_pendingBoardState;
    bool m_uploadFailed = false;
    
    // API routes
    const QString API_BASE_URL = "https://imagoref.ru/api";
    const QString WS_URL = "wss://imagoref.ru/ws";
};
