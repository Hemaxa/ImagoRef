#include "FileController.h"
#include "ImageModel.h"
#include "BoardController.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QBuffer>
#include <JlCompress.h>
#include <QTemporaryDir>
#include <QDir>
#include <QCryptographicHash>
#include "CacheManager.h"

FileController::FileController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent) : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
{}

QString FileController::getCurrentFilePath() const
{
    return m_currentFilePath;
}

//метод формирования заголовка окна
QString FileController::getWindowTitle() const
{
    QString title;
    if (m_currentFilePath.isEmpty()) {
        title = "Новая доска";
    }
    else {
        title = QFileInfo(m_currentFilePath).fileName();
    }
    return title;
}

void FileController::setGridSize(int gridSize)
{
    m_gridSize = gridSize;
}

void FileController::newBoard()
{
    m_model->clear();
    m_undoStack->clear();
    m_currentFilePath.clear();
    emit filePathChanged();
}

bool FileController::openBoard(const QUrl &fileUrl)
{
    //получение путя в системе
    QString filePath = fileUrl.toLocalFile();
    if (!QFile::exists(filePath)) return false;

    //создаем временную папку, куда будем распаковывать наш проект-архив
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return false;

    //распаковываем ZIP-архив (filePath) во временную папку (tempDir.path())
    if (JlCompress::extractDir(filePath, tempDir.path()).isEmpty()) {
        //игнорирование ошибки распаковки
    }

    QByteArray docData;
    QFile jsonFile(tempDir.path() + "/data.json");
    if (jsonFile.exists() && jsonFile.open(QIODevice::ReadOnly)) {
        docData = jsonFile.readAll();
        jsonFile.close();
    }
    else {
        //если файла data.json внутри нет (или это вообще был не архив), мы предполагаем, что пользователь пытается открыть старый проект из предыдущей версии приложения
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        docData = file.readAll();
        file.close();
    }

    //пытаемся превратить прочитанный текст в JSON документ
    QJsonDocument doc = QJsonDocument::fromJson(docData);
    if (doc.isNull()) return false;
    
    //перед загрузкой новой доски полностью очищаем старую
    m_model->clear();
    m_undoStack->clear();

    QJsonObject rootObj = doc.object();

    //загрузка настроек холста
    if (rootObj.contains("canvas")) {
        QJsonObject canvasObj = rootObj["canvas"].toObject();
        int loadedGridSize = canvasObj["gridSize"].toInt(25);
        m_gridSize = loadedGridSize;
        emit gridSizeLoaded(loadedGridSize);
        
        if (canvasObj.contains("cameraX") && canvasObj.contains("cameraY")) {
            qreal camX = canvasObj["cameraX"].toDouble();
            qreal camY = canvasObj["cameraY"].toDouble();
            qreal camZoom = canvasObj["cameraZoom"].toDouble(0.3);
            emit cameraLoaded(camX, camY, camZoom);
        }
    }

    //загрузка объектов
    if (rootObj.contains("items")) {
        QJsonArray itemsArray = rootObj["items"].toArray();
        for (const QJsonValue &value : itemsArray) {
            QJsonObject itemObj = value.toObject();

            QByteArray imageData;
            bool isOldFormat = false;
            if (itemObj.contains("imagePath")) {
                QString imagePath = itemObj["imagePath"].toString();
                QFile imgFile(tempDir.path() + "/" + imagePath);
                if (imgFile.open(QIODevice::ReadOnly)) {
                    imageData = imgFile.readAll();
                    imgFile.close();
                }
            } else if (itemObj.contains("imageData")) {
                imageData = QByteArray::fromBase64(itemObj["imageData"].toString().toLatin1());
                isOldFormat = true;
            }

            QImage image;
            if (image.loadFromData(imageData, "PNG")) {
                ImagoImageData data;
                data.id = itemObj["id"].toString();
                data.imageHash = QString(QCryptographicHash::hash(imageData, QCryptographicHash::Sha256).toHex());
                CacheManager::instance().saveToCache(data.imageHash, imageData);

                data.source = QUrl();
                data.pixmap = QPixmap::fromImage(image);
                data.x = itemObj["pos_x"].toDouble();
                data.y = itemObj["pos_y"].toDouble();
                data.width = itemObj["width"].toDouble();
                data.height = itemObj["height"].toDouble();
                data.rotation = itemObj["rotation"].toDouble();
                data.zValue = itemObj["zValue"].toDouble();
                data.label = itemObj["label"].toString();
                data.cropX = itemObj["cropX"].toDouble();
                data.cropY = itemObj["cropY"].toDouble();
                data.cropWidth = itemObj["cropWidth"].toDouble();
                data.cropHeight = itemObj["cropHeight"].toDouble();
                data.opacity = itemObj.contains("opacity") ? itemObj["opacity"].toDouble() : 1.0;
                
                m_model->addImage(data);
            }
        }
    }

    m_currentFilePath = filePath;
    emit filePathChanged();
    emit boardLoaded();
    return true;
}

//метод простого сохранения
bool FileController::saveBoard()
{
    //проверка наличия пути к файлу
    if (m_currentFilePath.isEmpty()) {
        return false;
    }
    //вызываем метод сохранения по пути с только что найденным нами путем
    return saveBoardAs(QUrl::fromLocalFile(m_currentFilePath));
}

bool FileController::saveBoardAs(const QUrl &fileUrl)
{
    //получение путя в системе
    QString filePath = fileUrl.toLocalFile();
    
    //создание главного JSON объекта с описанием проекта
    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    
    //создание вложенного объекта с настройками холста
    QJsonObject canvasObj;
    canvasObj["gridSize"] = m_gridSize;
    
    BoardController* board = qobject_cast<BoardController*>(parent());
    if (board) {
        //сохранение параметров камеры
        canvasObj["cameraX"] = board->getCameraX();
        canvasObj["cameraY"] = board->getCameraY();
        canvasObj["cameraZoom"] = board->getCameraZoom();
    }
    
    //включение вложенного объекта в основной
    rootObj["canvas"] = canvasObj;

    //создание временной папки
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) return false;

    //создание папки со всеми объектами
    QDir dir(tempDir.path());
    dir.mkdir("images");

    //создание массива с параметрами отдельной картинки
    QJsonArray itemsArray;
    
    //проход по всем картинкам и сохранение их параметров в массив
    for (const ImagoImageData &item : m_model->getAllItems()) {
        QJsonObject itemObj;
        itemObj["id"] = item.id;
        itemObj["pos_x"] = item.x;
        itemObj["pos_y"] = item.y;
        itemObj["width"] = item.width;
        itemObj["height"] = item.height;
        itemObj["rotation"] = item.rotation;
        itemObj["zValue"] = item.zValue;
        itemObj["label"] = item.label;
        itemObj["cropX"] = item.cropX;
        itemObj["cropY"] = item.cropY;
        itemObj["cropWidth"] = item.cropWidth;
        itemObj["cropHeight"] = item.cropHeight;
        itemObj["opacity"] = item.opacity;

        //относительный путь до картинки
        QString imageRelPath = QString("images/%1.png").arg(item.id);
        itemObj["imagePath"] = imageRelPath;
        
        //сохранение картинки из оперативной памяти
        QString imageAbsPath = dir.filePath(imageRelPath);
        item.pixmap.save(imageAbsPath, "PNG");

        //добавляем объект в массив
        itemsArray.append(itemObj);
    }

    //включение вложенного объекта в основной
    rootObj["items"] = itemsArray;

    //превращаем JSON-объект в текстовый документ QJsonDocument
    QJsonDocument doc(rootObj);

    //создаем файл "data.json" в корне нашей временной папки
    QFile jsonFile(dir.filePath("data.json"));

    //открываем файл для записи и записываем данные
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(doc.toJson());
        jsonFile.close();
    }
    else {
        return false;
    }

    //если по указанному пути уже есть файл, удаляем его, чтобы перезаписать
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    //сжимаем всю временную папку (с data.json и папкой images) в один ZIP-архив с помощью сторонней библиотеки JlCompress и сохраняем по пути filePath
    bool success = JlCompress::compressDir(filePath, tempDir.path());

    if (!success) {
        return false;
    }

    //обновляем текущий путь к файлу в памяти приложения
    m_currentFilePath = filePath;
    emit filePathChanged();
    emit boardSaved();
    return true;
}