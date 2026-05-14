//ToolController — контроллер инструментов. Отвечает за удаление, привязку к сетке, вращение, обрезку, подписи и автоматическое расположение элементов.

#pragma once

#include <QObject>
#include <QUndoStack>
#include <QtQml/qqml.h>

class ImagoImageModel;

class ToolController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("ToolController is only available via BoardController.toolController")

    //свойство включенного режима фиксации
    Q_PROPERTY(bool isPinned READ getIsPinned NOTIFY isPinnedChanged)
    Q_PROPERTY(bool isEyedropperActive READ getIsEyedropperActive NOTIFY isEyedropperActiveChanged)

public:
    explicit ToolController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    bool getIsPinned() const;
    bool getIsEyedropperActive() const;

    //методы активации соответствующих инструментов
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void snapToGrid();
    Q_INVOKABLE void rotateSelected(qreal angleDelta);
    Q_INVOKABLE void cropImage(int index, qreal cropX, qreal cropY, qreal cropWidth, qreal cropHeight);
    Q_INVOKABLE void setLabelForSelected(const QString &label);
    Q_INVOKABLE void setOpacityForSelected(qreal opacity);
    Q_INVOKABLE void arrangeAll(qreal centerX = 10000.0, qreal centerY = 10000.0);
    Q_INVOKABLE void togglePin();
    
    Q_INVOKABLE void toggleEyedropper();
    Q_INVOKABLE QColor getColorAtPoint(int globalX, int globalY);
    Q_INVOKABLE void copyColorToClipboard(const QColor &color);

signals:
    void selectionChanged();
    void isPinnedChanged();
    void isEyedropperActiveChanged();

private:
    ImagoImageModel *m_model;
    QUndoStack *m_undoStack;
    bool m_isPinned = false;
    bool m_isEyedropperActive = false;
};
