#pragma once

#include <QObject>
#include <QWebSocket>
#include <QString>

class ImagoImageModel;

class SyncController : public QObject {
    Q_OBJECT
public:
    explicit SyncController(ImagoImageModel *model, QObject *parent = nullptr);

    Q_INVOKABLE void connectToBoard(const QString &boardId);
    Q_INVOKABLE void disconnectFromBoard();

    void sendMoveEvent(const QString &itemId, qreal x, qreal y);
    void sendResizeEvent(const QString &itemId, qreal x, qreal y, qreal width, qreal height);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    ImagoImageModel *m_model;
    QWebSocket *m_webSocket;
    QString m_boardId;

    const QString WS_BASE_URL = "ws://imagoref.ru/ws/boards/";
};
