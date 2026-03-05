#pragma once

#include <QObject>
#include <QtQml/qqml.h>

class ImageItemModel;

/**
 * @brief SelectionController — контроллер выделения элементов на холсте.
 * Управляет выбором, снятием выбора, рамочным выделением и hit-тестом.
 */
class SelectionController : public QObject {
    Q_OBJECT
    QML_UNCREATABLE("SelectionController is only available via BoardController.selectionController")

    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)

public:
    explicit SelectionController(ImageItemModel *model, QObject *parent = nullptr);

    bool hasSelection() const;

    // Выделение
    Q_INVOKABLE void selectItem(int index, bool addToSelection = false);
    Q_INVOKABLE void deselectItem(int index);
    Q_INVOKABLE void toggleSelection(int index);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void selectInRect(qreal x, qreal y, qreal width, qreal height, bool addToSelection = false);

    // Hit-test
    Q_INVOKABLE int hitTest(qreal x, qreal y) const;

    // Аксессоры для QML
    Q_INVOKABLE qreal getItemX(int index) const;
    Q_INVOKABLE qreal getItemY(int index) const;
    Q_INVOKABLE qreal getItemWidth(int index) const;
    Q_INVOKABLE qreal getItemHeight(int index) const;
    Q_INVOKABLE bool isItemSelected(int index) const;

signals:
    void selectionChanged();

private:
    ImageItemModel *m_model;
};
