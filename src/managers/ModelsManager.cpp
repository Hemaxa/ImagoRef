#include "ModelsManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJSEngine>
#include <QQmlEngine>

const QString C_BASE_MODEL_URL = "https://imagoref.ru/models/";
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
    m_totalProgress = 0.0;
    emit downloadingChanged();
    emit progressChanged();

    m_downloadQueue.clear();
    m_downloadQueue << C_MODEL_BIN << C_MODEL_PARAM;
    m_fileIndex = 0;

    downloadNextFile();
}

void ModelsManager::downloadNextFile() {
    if (m_downloadQueue.isEmpty()) {
        m_isDownloading = false;
        emit downloadingChanged();
        checkModelExists();
        emit downloadFinished(true, "");
        return;
    }

    m_currentFileName = m_downloadQueue.takeFirst();
    QString urlStr = C_BASE_MODEL_URL + m_currentFileName;
    
    QNetworkRequest request{(QUrl(urlStr))};
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
        // Мы скачиваем 2 файла, поэтому каждый дает 50% прогресса
        qreal currentFileProgress = static_cast<qreal>(bytesReceived) / static_cast<qreal>(bytesTotal);
        m_downloadProgress = (m_fileIndex * 0.5) + (currentFileProgress * 0.5);
        emit progressChanged();
    }
}

void ModelsManager::onDownloadFinished() {
    if (m_networkReply->error() != QNetworkReply::NoError) {
        QString errorMsg = m_networkReply->errorString();
        m_networkReply->deleteLater();
        m_networkReply = nullptr;
        m_isDownloading = false;
        emit downloadingChanged();
        emit downloadFinished(false, errorMsg);
        return;
    }

    QString savePath = QDir(m_modelsDir).filePath(m_currentFileName);
    QFile targetFile(savePath);
    if (targetFile.open(QIODevice::WriteOnly)) {
        targetFile.write(m_networkReply->readAll());
        targetFile.close();
    } else {
        m_networkReply->deleteLater();
        m_networkReply = nullptr;
        m_isDownloading = false;
        emit downloadingChanged();
        emit downloadFinished(false, "Failed to save downloaded file " + m_currentFileName);
        return;
    }

    m_networkReply->deleteLater();
    m_networkReply = nullptr;
    
    m_fileIndex++;
    downloadNextFile();
}
