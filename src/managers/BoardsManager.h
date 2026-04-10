#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QtQml/qqml.h>
#include <QNetworkAccessManager>

class BoardsManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(QVariantList boards READ getBoards NOTIFY boardsChanged)

public:
    static BoardsManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static BoardsManager& instance();

    QVariantList getBoards() const;

    Q_INVOKABLE void loadBoards();
    Q_INVOKABLE void createBoard(const QString& title);
    Q_INVOKABLE void renameBoard(const QString& id, const QString& newTitle);
    Q_INVOKABLE void deleteBoard(const QString& id);

signals:
    void boardsChanged();
    void boardsFetched(bool success);
    void boardCreated(const QString& boardId, bool success);
    void boardRenamed(bool success);
    void boardDeleted(bool success);

private:
    explicit BoardsManager(QObject* parent = nullptr);
    BoardsManager(const BoardsManager&) = delete;
    BoardsManager& operator=(const BoardsManager&) = delete;

    QVariantList m_boards;
    QNetworkAccessManager m_networkManager;
};
