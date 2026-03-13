//BoardController — фасад-контроллер приложения для QML. Координирует подконтроллеры и хранит общие ресурсы (модель, undo-стек)

#pragma once

#include <QObject>
#include <QUrl>
#include <QUndoStack>
#include <QtQml/qqml.h>

#include "ImageModel.h"
#include "FileController.h"
#include "SelectionController.h"
#include "ClipboardController.h"
#include "ToolController.h"
#include "UpscaleController.h"

class BoardController : public QObject {
    Q_OBJECT //обязательный макрос для любого класса Qt, который использует сигналы, слоты или свойства (Q_PROPERTY)
    QML_ELEMENT //макрос, который позволяет использовать этот класс напрямую в QML

    //свойство Q_PROPERTY делает переменные C++ доступными в QML как обычные свойства
    Q_PROPERTY(ImagoImageModel* model READ getModel CONSTANT) //модель данных
    
    //остальные контроллеры
    Q_PROPERTY(FileController* fileController READ getFileController CONSTANT)
    Q_PROPERTY(SelectionController* selectionController READ getSelectionController CONSTANT)
    Q_PROPERTY(ClipboardController* clipboardController READ getClipboardController CONSTANT)
    Q_PROPERTY(ToolController* toolController READ getToolController CONSTANT)
    Q_PROPERTY(UpscaleController* upscaleController READ getUpscaleController CONSTANT)

    //состояния для кнопок Undo/Redo
    Q_PROPERTY(bool canUndo READ getCanUndo NOTIFY undoStateChanged)
    Q_PROPERTY(bool canRedo READ getCanRedo NOTIFY redoStateChanged)
    
    //размер сетки
    Q_PROPERTY(int gridSize READ getGridSize WRITE setGridSize NOTIFY gridSizeChanged)

    Q_PROPERTY(qreal cameraX READ getCameraX WRITE setCameraX NOTIFY cameraChanged)
    Q_PROPERTY(qreal cameraY READ getCameraY WRITE setCameraY NOTIFY cameraChanged)
    Q_PROPERTY(qreal cameraZoom READ getCameraZoom WRITE setCameraZoom NOTIFY cameraChanged)

public:
    //конструктор принимает родительский QObject для автоматического управления памятью
    explicit BoardController(QObject *parent = nullptr);
    ~BoardController();

    //геттеры для свойств
    ImagoImageModel* getModel() const;
    FileController* getFileController() const;
    SelectionController* getSelectionController() const;
    ClipboardController* getClipboardController() const;
    ToolController* getToolController() const;
    UpscaleController* getUpscaleController() const;

    bool getCanUndo() const;
    bool getCanRedo() const;
    int getGridSize() const;
    qreal getCameraX() const;
    qreal getCameraY() const;
    qreal getCameraZoom() const;

    //сеттеры для свойств
    void setGridSize(int size);
    void setCameraX(qreal x);
    void setCameraY(qreal y);
    void setCameraZoom(qreal zoom);

    //свойство Q_INVOKABLE позволяет вызывать эти C++ методы прямо из JavaScript/QML кода
    //Undo/Redo
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    //отслеживание перемещения/ресайза для Undo
    Q_INVOKABLE void beginMove(int index);
    Q_INVOKABLE void endMove(int index, qreal newX, qreal newY);
    Q_INVOKABLE void beginResize(int index);
    Q_INVOKABLE void endResize(int index, qreal newX, qreal newY, qreal newWidth, qreal newHeight);

    //отслеживание перемещения нескольких элементов (выделения)
    Q_INVOKABLE void beginMoveSelection();
    Q_INVOKABLE void updateMoveSelection(qreal deltaX, qreal deltaY);
    Q_INVOKABLE void endMoveSelection();

signals:
    //сигналы, которые оповещают QML об изменениях (связаны с макросами NOTIFY в Q_PROPERTY)
    void undoStateChanged();
    void redoStateChanged();
    void gridSizeChanged();
    void cameraChanged();

private:
    //вспомогательный метод для настройки сигналов
    void connectSignals();

    //внутренние переменные класса
    ImagoImageModel *m_model;
    QUndoStack *m_undoStack;
    int m_gridSize;
    qreal m_cameraX;
    qreal m_cameraY;
    qreal m_cameraZoom;

    //указатели на остальные контроллеры
    FileController *m_fileController;
    SelectionController *m_selectionController;
    ClipboardController *m_clipboardController;
    ToolController *m_toolController;
    UpscaleController *m_upscaleController;

    //переменные для хранения начального состояния объекта, когда пользователь только начинает его перетаскивать или менять размер
    QPointF m_moveStartPos;
    QRectF m_resizeStartRect;
    QPointF m_resizeStartPos;

    //переменные для хранения начального состояния группы элементов при перетаскивании выделения
    QVector<int> m_moveSelectionIndices;
    QVector<QPointF> m_moveSelectionStartPos;
};
