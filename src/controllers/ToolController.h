#pragma once

#include <QObject>
#include <QUndoStack>
#include <QtQml/qqml.h>

class ImageItemModel;

/**
 * @brief ToolController — контроллер инструментов.
 * Отвечает за удаление, привязку к сетке, вращение, обрезку,
 * подписи и автоматическое расположение элементов.
 */
class ToolController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("ToolController is only available via BoardController.toolController")

public:
    explicit ToolController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent = nullptr);

    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void snapToGrid();
    Q_INVOKABLE void rotateSelected(qreal angleDelta);
    Q_INVOKABLE void cropImage(int index, qreal cropX, qreal cropY, qreal cropWidth, qreal cropHeight);
    Q_INVOKABLE void setLabelForSelected(const QString &label);
    Q_INVOKABLE void arrangeAll();

signals:
    void selectionChanged();

private:
    ImageItemModel *m_model;
    QUndoStack *m_undoStack;
};
