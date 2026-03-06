//SelectionController — контроллер выделения элементов на холсте. Управляет выбором, снятием выбора, рамочным выделением и нажатием ЛКМ по одной картинке

#pragma once

#include <QObject>
#include <QtQml/qqml.h>

class ImageItemModel; //предварительное объявление класса

class SelectionController : public QObject {
    Q_OBJECT
    //этот макрос запрещает создавать этот класс прямо из QML, вместо этого он должен использовать только тот экземпляр из BoardController
    QML_UNCREATABLE("SelectionController is only available via BoardController.selectionController")

    //свойство активного/неактивного выбора картинки
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)

public:
    explicit SelectionController(ImageItemModel *model, QObject *parent = nullptr);

    //метод, который проверяет, есть ли хоть одна выделенная картинка
    bool hasSelection() const;

    //выделение
    Q_INVOKABLE void selectItem(int index, bool addToSelection = false);
    Q_INVOKABLE void deselectItem(int index);
    Q_INVOKABLE void toggleSelection(int index);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void selectInRect(qreal x, qreal y, qreal width, qreal height, bool addToSelection = false);

    //Hit-test
    Q_INVOKABLE int hitTest(qreal x, qreal y) const;

    //вспомогательные методы, чтобы QML мог быстро узнать геометрию выделенного элемента, не обращаясь напрямую к тяжелой модели данных.
    Q_INVOKABLE qreal getItemX(int index) const;
    Q_INVOKABLE qreal getItemY(int index) const;
    Q_INVOKABLE qreal getItemWidth(int index) const;
    Q_INVOKABLE qreal getItemHeight(int index) const;
    Q_INVOKABLE bool isItemSelected(int index) const;

signals:
    //сигнал, который отправляется при любом изменении списка выделенных элементов
    void selectionChanged();

private:
    //внутренние переменные класса
    ImageItemModel *m_model;
};
