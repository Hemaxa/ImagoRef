#include "CloudBoardsManager.h"
#include "SettingsManager.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJSEngine>

CloudBoardsManager* CloudBoardsManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

CloudBoardsManager& CloudBoardsManager::instance()
{
    static CloudBoardsManager manager;
    return manager;
}

CloudBoardsManager::CloudBoardsManager(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

QVariantList CloudBoardsManager::getCloudBoards() const
{
    return m_cloudBoards;
}

QNetworkRequest CloudBoardsManager::createRequest(const QString& endpoint) const
{
    QNetworkRequest request(QUrl(API_BASE_URL + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString token = SettingsManager::instance().getJwtToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }
    return request;
}

void CloudBoardsManager::fetchBoards()
{
    QNetworkRequest request = createRequest("/boards/");
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &CloudBoardsManager::onFetchBoardsReply);
}

void CloudBoardsManager::onFetchBoardsReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit boardsFetched(false);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray arr = doc.array();

    m_cloudBoards.clear();
    for (int i = 0; i < arr.size(); ++i) {
        QJsonObject obj = arr[i].toObject();
        QVariantMap map;
        map["id"] = QString::number(obj["id"].toInt());
        map["name"] = obj["title"].toString();
        // Можно добавить previewHash или дату 
        m_cloudBoards.append(map);
    }

    emit cloudBoardsChanged();
    emit boardsFetched(true);
}

void CloudBoardsManager::createBoard(const QString& title)
{
    QJsonObject reqObj;
    reqObj["title"] = title;

    QNetworkRequest request = createRequest("/boards/");
    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(reqObj).toJson());
    connect(reply, &QNetworkReply::finished, this, &CloudBoardsManager::onCreateBoardReply);
}

void CloudBoardsManager::onCreateBoardReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit boardCreated("", false);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QString id = QString::number(obj["id"].toInt());

    emit boardCreated(id, true);
    fetchBoards(); // Обновляем список
}

void CloudBoardsManager::renameBoard(const QString& id, const QString& newTitle)
{
    QJsonObject reqObj;
    reqObj["title"] = newTitle;

    QNetworkRequest request = createRequest("/boards/" + id);
    QNetworkReply *reply = m_networkManager->put(request, QJsonDocument(reqObj).toJson());
    connect(reply, &QNetworkReply::finished, this, &CloudBoardsManager::onRenameBoardReply);
}

void CloudBoardsManager::onRenameBoardReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit boardRenamed(false);
        return;
    }

    emit boardRenamed(true);
    fetchBoards(); // Обновляем список
}

void CloudBoardsManager::deleteBoard(const QString& id)
{
    QNetworkRequest request = createRequest("/boards/" + id);
    QNetworkReply *reply = m_networkManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, &CloudBoardsManager::onDeleteBoardReply);
}

void CloudBoardsManager::onDeleteBoardReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit boardDeleted(false);
        return;
    }

    emit boardDeleted(true);
    fetchBoards(); // Обновляем список
}
