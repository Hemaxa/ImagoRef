//StorageController — контроллер файловых операций и БД. Отвечает за локальное хранение данных и экспорт/импорт досок (.iref)

#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantList>
#include "ImageModel.h"

class ImagoImageModel;

class StorageController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("StorageController is only available via BoardController.storageController")

    //свойство проверки наличия файлового пути (новая доска или открытый файл)
    Q_PROPERTY(QString currentFilePath READ getCurrentFilePath NOTIFY filePathChanged)
    //свойство генерации заголовка окна в зависимости от файла
    Q_PROPERTY(QString windowTitle READ getWindowTitle NOTIFY filePathChanged)

public:
    explicit StorageController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent = nullptr);
    ~StorageController();

    // Статические методы для работы с БД (для BoardsManager)
    static void initDatabase();
    static QVariantList getLocalBoards();
    static void createLocalBoard(const QString& id, const QString& title);
    static void renameLocalBoard(const QString& id, const QString& newTitle);
    static void deleteLocalBoard(const QString& id);

    //геттеры
    QString getCurrentFilePath() const;
    QString getWindowTitle() const;
    QString getBoardTitle(const QString& boardId);

    //методы синхронизации и атомарных сохранений
    void upsertItem(const ImagoImageData &item);
    void deleteItem(const QString &itemId);
    void enqueueSyncTask(const QString &actionType, const QJsonObject &payload);
    void updateBoardMetadata(qreal camX, qreal camY, qreal camZoom);
    void loadBoardFromDb(const QString& boardId);

    QVariantList getSyncTasks(int limit = 5);
    void removeSyncTask(int taskId);
    bool applyNetworkDelta(const QString& actionType, const QJsonObject& payload);
    ImagoImageData getItemFromDb(const QString& itemId);
    bool isLoading() const { return m_isLoading; }

    //методы операций
    Q_INVOKABLE void newBoard();
    Q_INVOKABLE bool openBoard(const QUrl &fileUrl);
    Q_INVOKABLE bool saveBoard();
    Q_INVOKABLE bool saveBoardAs(const QUrl &fileUrl);

    //сеттеры
    void setGridSize(int gridSize);
    
signals:
    //сигналы для обновления интерфейса
    void filePathChanged();
    void boardLoaded();
    void boardSaved();

    //вызывается при загрузке доски с сохранённым gridSize
    void gridSizeLoaded(int gridSize);
    void cameraLoaded(qreal x, qreal y, qreal zoom);

private:
    bool importFromIref(const QString& filePath);
    bool exportToIref(const QString& boardId, const QString& filePath);

    //внутренние поля класса
    ImagoImageModel *m_model;
    QUndoStack *m_undoStack;
    QString m_currentFilePath;
    int m_gridSize = 25;
    bool m_isLoading = false;
};
