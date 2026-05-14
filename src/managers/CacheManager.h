#pragma once

#include <QObject>
#include <QString>
#include <QPixmap>

class CacheManager : public QObject {
    Q_OBJECT

public:
    static CacheManager& instance();

    bool isCached(const QString &hash) const;
    void saveToCache(const QString &hash, const QPixmap &pixmap);
    void saveToCache(const QString &hash, const QByteArray &data);
    QPixmap loadFromCache(const QString &hash) const;
    QString getCacheFilePath(const QString &hash) const;

private:
    explicit CacheManager(QObject *parent = nullptr);
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;

    QString m_cacheDir;
};
