#include "SyncController.h"
#include "ImageModel.h"
#include "SettingsManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>

SyncController::SyncController(ImagoImageModel *model, QObject *parent)
    : QObject(parent), m_model(model), m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    connect(m_webSocket, &QWebSocket::connected, this, &SyncController::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &SyncController::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &SyncController::onTextMessageReceived);
}

void SyncController::connectToBoard(const QString &boardId)
{
    m_boardId = boardId;
    QUrl url(WS_BASE_URL + boardId);

    QString token = SettingsManager::instance().getJwtToken();
    if (!token.isEmpty()) {
        QUrlQuery query;
        query.addQueryItem("token", token);
        url.setQuery(query);
    }

    m_webSocket->open(url);
}

void SyncController::disconnectFromBoard()
{
    m_boardId.clear();
    m_webSocket->close();
}

void SyncController::sendMoveEvent(const QString &itemId, qreal x, qreal y)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) return;

    QJsonObject obj;
    obj["action"] = "move";
    obj["item_id"] = itemId;
    obj["x"] = x;
    obj["y"] = y;

    m_webSocket->sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SyncController::sendResizeEvent(const QString &itemId, qreal x, qreal y, qreal width, qreal height)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) return;

    QJsonObject obj;
    obj["action"] = "resize";
    obj["item_id"] = itemId;
    obj["x"] = x;
    obj["y"] = y;
    obj["width"] = width;
    obj["height"] = height;

    m_webSocket->sendTextMessage(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void SyncController::onConnected()
{
}

void SyncController::onDisconnected()
{
    if (!m_boardId.isEmpty()) {
        QTimer::singleShot(5000, this, [this]() {
            if (!m_boardId.isEmpty() && m_webSocket->state() == QAbstractSocket::UnconnectedState) {
                connectToBoard(m_boardId);
            }
        });
    }
}

void SyncController::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();

    QString action = obj["action"].toString();
    QString itemId = obj["item_id"].toString();

    int index = m_model->getIndexById(itemId);
    if (index < 0) return; // Объект не найден

    if (action == "move") {
        qreal x = obj["x"].toDouble();
        qreal y = obj["y"].toDouble();
        m_model->setPosition(index, x, y);
    } else if (action == "resize") {
        qreal x = obj["x"].toDouble();
        qreal y = obj["y"].toDouble();
        qreal w = obj["width"].toDouble();
        qreal h = obj["height"].toDouble();
        
        m_model->setPosition(index, x, y);
        m_model->setSize(index, w, h);
    }
}
