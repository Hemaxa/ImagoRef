#include "CacheManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QFile>

CacheManager& CacheManager::instance() {
    static CacheManager instance;
    return instance;
}

CacheManager::CacheManager(QObject *parent) : QObject(parent) {
    m_cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/image_cache";
    QDir dir;
    if (!dir.exists(m_cacheDir)) {
        dir.mkpath(m_cacheDir);
    }
}

bool CacheManager::isCached(const QString &hash) const {
    if (hash.isEmpty()) return false;
    return QFileInfo::exists(getCacheFilePath(hash));
}

QString CacheManager::getCacheFilePath(const QString &hash) const {
    return m_cacheDir + "/" + hash + ".png";
}

void CacheManager::saveToCache(const QString &hash, const QPixmap &pixmap) {
    if (hash.isEmpty() || pixmap.isNull()) return;
    QString path = getCacheFilePath(hash);
    if (!QFileInfo::exists(path)) {
        pixmap.save(path, "PNG");
    }
}

void CacheManager::saveToCache(const QString &hash, const QByteArray &data) {
    if (hash.isEmpty() || data.isEmpty()) return;
    QString path = getCacheFilePath(hash);
    if (!QFileInfo::exists(path)) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
        }
    }
}

QPixmap CacheManager::loadFromCache(const QString &hash) const {
    if (isCached(hash)) {
        return QPixmap(getCacheFilePath(hash));
    }
    return QPixmap();
}
