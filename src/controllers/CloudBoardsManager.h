#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QVariantList>
#include <QtQml/qqml.h>

class CloudBoardsManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(QVariantList cloudBoards READ getCloudBoards NOTIFY cloudBoardsChanged)

public:
    static CloudBoardsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static CloudBoardsManager& instance();

    QVariantList getCloudBoards() const;

    Q_INVOKABLE void fetchBoards();
    Q_INVOKABLE void createBoard(const QString& title);
    Q_INVOKABLE void renameBoard(const QString& id, const QString& newTitle);
    Q_INVOKABLE void deleteBoard(const QString& id);

signals:
    void cloudBoardsChanged();
    void boardsFetched(bool success);
    void boardCreated(const QString& id, bool success);
    void boardRenamed(bool success);
    void boardDeleted(bool success);

private slots:
    void onFetchBoardsReply();
    void onCreateBoardReply();
    void onRenameBoardReply();
    void onDeleteBoardReply();

private:
    explicit CloudBoardsManager(QObject* parent = nullptr);
    CloudBoardsManager(const CloudBoardsManager&) = delete;
    CloudBoardsManager& operator=(const CloudBoardsManager&) = delete;

    QNetworkRequest createRequest(const QString& endpoint) const;

    QNetworkAccessManager *m_networkManager;
    QVariantList m_cloudBoards;
    
    const QString API_BASE_URL = "https://imagoref.ru/api";
};
