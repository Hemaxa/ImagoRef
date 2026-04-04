#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QtQml/qqml.h>

class AuthController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(bool isLoggedIn READ getIsLoggedIn NOTIFY authStateChanged)
    Q_PROPERTY(QString userEmail READ getUserEmail NOTIFY authStateChanged)
    Q_PROPERTY(QString userNickname READ getUserNickname NOTIFY authStateChanged)
    Q_PROPERTY(QString userAvatarHash READ getUserAvatarHash NOTIFY authStateChanged)

public:
    static AuthController* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static AuthController& instance();

    Q_INVOKABLE void login(const QString& email, const QString& password);
    Q_INVOKABLE void registerUser(const QString& email, const QString& password);
    Q_INVOKABLE void logout();

    bool getIsLoggedIn() const;
    QString getUserEmail() const;
    QString getUserNickname() const;
    QString getUserAvatarHash() const;
    
    Q_INVOKABLE void updateProfile(const QString &nickname, const QString &avatarFilePath);

signals:
    void authStateChanged();
    void loginFinished(bool success, const QString& message);
    void registerFinished(bool success, const QString& message);

private slots:
    void onLoginReply();
    void onRegisterReply();

private:
    explicit AuthController(QObject* parent = nullptr);
    AuthController(const AuthController&) = delete;
    AuthController& operator=(const AuthController&) = delete;

    QNetworkAccessManager *m_networkManager;
    const QString API_BASE_URL = "https://imagoref.ru/api";
};
