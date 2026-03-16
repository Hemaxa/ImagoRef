#include "ModelsManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJSEngine>
#include <QQmlEngine>
#include <quazip/JlCompress.h>

const QString C_MODEL_URL = "https://imagoref.ru/models/upscale-model.zip";
const QString C_MODEL_BIN = "model-DF2K.bin";
const QString C_MODEL_PARAM = "model-DF2K.param";

ModelsManager* ModelsManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

ModelsManager& ModelsManager::instance()
{
    static ModelsManager manager;
    return manager;
}

ModelsManager::ModelsManager(QObject *parent) : QObject(parent), m_networkReply(nullptr) {
    m_networkManager = new QNetworkAccessManager(this);
    
    // Setup models directory in AppData folder
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_modelsDir = QDir(appDataDir).filePath("models");
    
    // Ensure the models directory exists
    QDir().mkpath(m_modelsDir);
    
    checkModelExists();
}

bool ModelsManager::isModelDownloaded() const { return m_isModelDownloaded; }
bool ModelsManager::isDownloading() const { return m_isDownloading; }
qreal ModelsManager::downloadProgress() const { return m_downloadProgress; }

QString ModelsManager::getModelPath() const {
    return QDir(m_modelsDir).filePath(C_MODEL_BIN);
}

QString ModelsManager::getParamPath() const {
    return QDir(m_modelsDir).filePath(C_MODEL_PARAM);
}

void ModelsManager::checkModelExists() {
    bool exists = QFile::exists(getModelPath()) && QFile::exists(getParamPath());
    if (m_isModelDownloaded != exists) {
        m_isModelDownloaded = exists;
        emit modelDownloadedChanged();
    }
}

void ModelsManager::downloadModel() {
    if (m_isDownloading || m_isModelDownloaded) return;

    m_isDownloading = true;
    m_downloadProgress = 0.0;
    emit downloadingChanged();
    emit progressChanged();

    QNetworkRequest request{(QUrl(C_MODEL_URL))};
    // Follow redirects
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    m_networkReply = m_networkManager->get(request);
    connect(m_networkReply, &QNetworkReply::downloadProgress, this, &ModelsManager::onDownloadProgress);
    connect(m_networkReply, &QNetworkReply::finished, this, &ModelsManager::onDownloadFinished);
}

void ModelsManager::deleteModel() {
    if (m_isDownloading) return;
    
    QFile::remove(getModelPath());
    QFile::remove(getParamPath());
    
    checkModelExists();
}

void ModelsManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        m_downloadProgress = static_cast<qreal>(bytesReceived) / static_cast<qreal>(bytesTotal);
        emit progressChanged();
    }
}

void ModelsManager::onDownloadFinished() {
    m_isDownloading = false;
    emit downloadingChanged();

    if (m_networkReply->error() != QNetworkReply::NoError) {
        QString errorMsg = m_networkReply->errorString();
        m_networkReply->deleteLater();
        m_networkReply = nullptr;
        emit downloadFinished(false, errorMsg);
        return;
    }

    // Save zip to a temporary file
    QString zipPath = QDir(m_modelsDir).filePath("upscale-model.zip");
    QFile zipFile(zipPath);
    if (zipFile.open(QIODevice::WriteOnly)) {
        zipFile.write(m_networkReply->readAll());
        zipFile.close();
        
        extractArchive(zipPath);
        QFile::remove(zipPath); // Clean up zip
        
        checkModelExists();
        emit downloadFinished(true, "");
    } else {
        emit downloadFinished(false, "Failed to save downloaded file.");
    }

    m_networkReply->deleteLater();
    m_networkReply = nullptr;
}

void ModelsManager::extractArchive(const QString &zipPath) {
    if (!QFile::exists(zipPath)) return;
    
    // Extract using QuaZip
    JlCompress::extractDir(zipPath, m_modelsDir);
}
