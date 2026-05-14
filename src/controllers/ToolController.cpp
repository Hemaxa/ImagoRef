#include "ToolController.h"
#include "ImageModel.h"
#include "StackController.h"
#include "SettingsManager.h"

#include <cmath>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include <QClipboard>
#include <QColor>

ToolController::ToolController(ImagoImageModel *model, QUndoStack *undoStack, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
    , m_isPinned(false)
    , m_isEyedropperActive(false)
{
}

bool ToolController::getIsPinned() const
{
    return m_isPinned;
}

void ToolController::togglePin()
{
    m_isPinned = !m_isPinned;
    emit isPinnedChanged();
}

bool ToolController::getIsEyedropperActive() const
{
    return m_isEyedropperActive;
}

void ToolController::toggleEyedropper()
{
    m_isEyedropperActive = !m_isEyedropperActive;
    emit isEyedropperActiveChanged();
}

QColor ToolController::getColorAtPoint(int globalX, int globalY)
{
    QScreen *screen = QGuiApplication::screenAt(QPoint(globalX, globalY));
    if (!screen) screen = QGuiApplication::primaryScreen();
    if (!screen) return Qt::black;

    // Снимок экрана 1x1 пиксель
    QPixmap pixmap = screen->grabWindow(0, globalX, globalY, 1, 1);
    QImage image = pixmap.toImage();
    if (image.isNull()) return Qt::black;
    return image.pixelColor(0, 0);
}

void ToolController::copyColorToClipboard(const QColor &color)
{
    if (!color.isValid()) return;
    
    int mode = SettingsManager::instance().getColorCopyMode();
    QString colorText;
    
    if (mode == 0) { // HEX
        colorText = color.name(QColor::HexRgb).toUpper();
    } else { // RGB
        colorText = QString("rgb(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
    }
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(colorText);
    
    // Сохраняем в историю
    SettingsManager::instance().addColorToHistory(color.name(QColor::HexRgb).toUpper());
}

void ToolController::deleteSelected()
{
    QVariantList indices = m_model->getSelectedIndices();
    if (indices.isEmpty()) return;

    QList<int> intIndices;
    for (const QVariant &v : indices) {
        intIndices.append(v.toInt());
    }

    m_undoStack->push(new RemoveImageCommand(m_model, intIndices));
    emit selectionChanged();
}

void ToolController::snapToGrid()
{
    int gridSize = SettingsManager::instance().getGridSize();
    if (gridSize <= 0) return;

    QVariantList indices = m_model->getSelectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro("Привязка к сетке");

    for (const QVariant &v : indices) {
        int i = v.toInt();
        ImagoImageData data = m_model->getItem(i);
        
        qreal oldX = data.x;
        qreal oldY = data.y;
        
        qreal cx = oldX + data.width / 2.0;
        qreal cy = oldY + data.height / 2.0;

        qreal rad = data.rotation * M_PI / 180.0;
        qreal cosA = std::cos(rad);
        qreal sinA = std::sin(rad);

        qreal rx = (-data.width / 2.0) * cosA - (-data.height / 2.0) * sinA;
        qreal ry = (-data.width / 2.0) * sinA + (-data.height / 2.0) * cosA;

        qreal targetX = cx + rx;
        qreal targetY = cy + ry;

        qreal snappedX = std::round(targetX / gridSize) * gridSize;
        qreal snappedY = std::round(targetY / gridSize) * gridSize;

        qreal newCx = snappedX - rx;
        qreal newCy = snappedY - ry;

        qreal newX = newCx - data.width / 2.0;
        qreal newY = newCy - data.height / 2.0;

        if (!qFuzzyCompare(oldX, newX) || !qFuzzyCompare(oldY, newY)) {
            m_undoStack->push(new MoveImageCommand(
                m_model, i, 
                QPointF(oldX, oldY),
                QPointF(newX, newY)
            ));
        }
    }

    m_undoStack->endMacro();
}

void ToolController::rotateSelected(qreal angleDelta)
{
    QVariantList indices = m_model->getSelectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro(angleDelta > 0 ? "Вращение по часовой" : "Вращение против часовой");

    for (const QVariant &v : indices) {
        int idx = v.toInt();
        m_undoStack->push(new RotateImageCommand(m_model, idx, angleDelta));
    }

    m_undoStack->endMacro();
}

void ToolController::cropImage(int index, qreal cropX, qreal cropY, qreal cropWidth, qreal cropHeight)
{
    if (index < 0 || index >= m_model->getCount()) return;
    
    ImagoImageData item = m_model->getItem(index);
    if (item.pixmap.isNull()) return;
    
    qreal sourceWidth = (item.cropWidth > 0) ? item.cropWidth : item.pixmap.width();
    qreal sourceHeight = (item.cropHeight > 0) ? item.cropHeight : item.pixmap.height();
    
    // Scale from scene dimensions back to the current pixmap dimensions
    qreal scaleX = sourceWidth / item.width;
    qreal scaleY = sourceHeight / item.height;
    
    // Calculate new crop relative to the current pixmap
    
    // Update crop inside the pixmap bounds. If it already had a crop, we add to it.
    qreal currentSourceX = (item.cropWidth > 0) ? item.cropX : 0;
    qreal currentSourceY = (item.cropHeight > 0) ? item.cropY : 0;
    
    qreal newSourceCropX = currentSourceX + (cropX * scaleX);
    qreal newSourceCropY = currentSourceY + (cropY * scaleY);
    qreal newSourceCropW = cropWidth * scaleX;
    qreal newSourceCropH = cropHeight * scaleY;
    
    qreal cropCenterLocalX = cropX + cropWidth / 2.0;
    qreal cropCenterLocalY = cropY + cropHeight / 2.0;
    
    qreal oldCenterX = item.width / 2.0;
    qreal oldCenterY = item.height / 2.0;
    
    qreal relX = cropCenterLocalX - oldCenterX;
    qreal relY = cropCenterLocalY - oldCenterY;
    
    qreal rad = item.rotation * M_PI / 180.0;
    qreal rotatedRelX = relX * std::cos(rad) - relY * std::sin(rad);
    qreal rotatedRelY = relX * std::sin(rad) + relY * std::cos(rad);
    
    qreal sceneCropCenterX = (item.x + oldCenterX) + rotatedRelX;
    qreal sceneCropCenterY = (item.y + oldCenterY) + rotatedRelY;
    
    qreal newX = sceneCropCenterX - cropWidth / 2.0;
    qreal newY = sceneCropCenterY - cropHeight / 2.0;
    
    m_undoStack->push(new CropImageCommand(
        m_model, index,
        QPointF(item.x, item.y), QSizeF(item.width, item.height),
        QRectF(item.cropX, item.cropY, item.cropWidth, item.cropHeight),
        QPointF(newX, newY), QSizeF(cropWidth, cropHeight),
        QRectF(newSourceCropX, newSourceCropY, newSourceCropW, newSourceCropH)
    ));
}

void ToolController::setLabelForSelected(const QString &label)
{
    QVariantList indices = m_model->getSelectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro("Подписать изображения");

    for (const QVariant &v : indices) {
        int idx = v.toInt();
        ImagoImageData item = m_model->getItem(idx);
        if (item.label != label) {
            m_undoStack->push(new SetLabelCommand(m_model, idx, item.label, label));
        }
    }

    m_undoStack->endMacro();
}

void ToolController::setOpacityForSelected(qreal opacity)
{
    QVariantList indices = m_model->getSelectedIndices();
    if (indices.isEmpty()) return;

    QVector<int> intIndices;
    QVector<qreal> oldOpacities;
    bool needsChange = false;
    
    for (const QVariant &v : indices) {
        int idx = v.toInt();
        qreal oldOp = m_model->getItem(idx).opacity;
        intIndices.append(idx);
        oldOpacities.append(oldOp);
        
        if (!qFuzzyCompare(oldOp, opacity)) {
            needsChange = true;
        }
    }

    if (!needsChange) return;

    m_undoStack->push(new SetOpacityCommand(m_model, intIndices, oldOpacities, opacity));
}

void ToolController::arrangeAll(qreal centerX, qreal centerY)
{
    int count = m_model->getCount();
    if (count == 0) return;

    int spacing = SettingsManager::instance().getArrangeSpacing();

    struct ItemInfo {
        int index;
        qreal itemWidth;
        qreal itemHeight;
        qreal bbWidth;
        qreal bbHeight;
    };
    
    QVector<ItemInfo> items;
    items.reserve(count);
    qreal totalArea = 0;
    
    for (int i = 0; i < count; ++i) {
        ImagoImageData data = m_model->getItem(i);
        
        qreal rad = std::fabs(data.rotation) * M_PI / 180.0;
        qreal cosA = std::fabs(std::cos(rad));
        qreal sinA = std::fabs(std::sin(rad));
        qreal bbW = data.width * cosA + data.height * sinA;
        qreal bbH = data.width * sinA + data.height * cosA;
        
        items.append({i, data.width, data.height, bbW, bbH});
        totalArea += (bbW + spacing) * (bbH + spacing);
    }

    std::sort(items.begin(), items.end(), [](const ItemInfo &a, const ItemInfo &b) {
        return a.bbHeight > b.bbHeight;
    });

    qreal maxRowWidth = std::sqrt(totalArea) * 1.3;
    if (maxRowWidth < 800) maxRowWidth = 800;

    qreal startX = centerX - maxRowWidth / 2.0;
    
    qreal totalHeight = 0;
    {
        qreal cx = 0, rh = 0;
        for (const auto &item : items) {
            if (cx + item.bbWidth > maxRowWidth && cx > 0) {
                totalHeight += rh + spacing;
                cx = 0;
                rh = 0;
            }
            cx += item.bbWidth + spacing;
            rh = std::max(rh, item.bbHeight);
        }
        totalHeight += rh;
    }
    qreal startY = centerY - totalHeight / 2.0;

    QVector<int> sortedIndices;
    QVector<QPointF> oldPositions;
    QVector<QPointF> newPositions;

    qreal currentX = startX;
    qreal currentY = startY;
    qreal rowHeight = 0;

    for (const auto &item : items) {
        if (currentX - startX + item.bbWidth > maxRowWidth && currentX > startX) {
            currentX = startX;
            currentY += rowHeight + spacing;
            rowHeight = 0;
        }

        qreal newX = currentX + item.bbWidth / 2.0 - item.itemWidth / 2.0;
        qreal newY = currentY + item.bbHeight / 2.0 - item.itemHeight / 2.0;

        ImagoImageData data = m_model->getItem(item.index);
        sortedIndices.append(item.index);
        oldPositions.append(QPointF(data.x, data.y));
        newPositions.append(QPointF(newX, newY));

        currentX += item.bbWidth + spacing;
        rowHeight = std::max(rowHeight, item.bbHeight);
    }

    m_undoStack->push(new ArrangeCommand(m_model, sortedIndices, oldPositions, newPositions));
}
