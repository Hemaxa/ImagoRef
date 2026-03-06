#include "ToolController.h"
#include "ImageModel.h"
#include "StackController.h"
#include "SettingsManager.h"

#include <cmath>

ToolController::ToolController(ImageItemModel *model, QUndoStack *undoStack, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_undoStack(undoStack)
    , m_isPinned(false)
{
}

bool ToolController::isPinned() const
{
    return m_isPinned;
}

void ToolController::togglePin()
{
    m_isPinned = !m_isPinned;
    emit isPinnedChanged();
}

void ToolController::deleteSelected()
{
    QVariantList indices = m_model->selectedIndices();
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
    int gridSize = SettingsManager::instance().gridSize();
    if (gridSize <= 0) return;

    QVariantList indices = m_model->selectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro("Привязка к сетке");

    for (const QVariant &v : indices) {
        int i = v.toInt();
        ImageData item = m_model->getItem(i);
        qreal newX = std::round(item.x / gridSize) * gridSize;
        qreal newY = std::round(item.y / gridSize) * gridSize;

        if (item.x != newX || item.y != newY) {
            m_undoStack->push(new MoveImageCommand(
                m_model, i, 
                QPointF(item.x, item.y),
                QPointF(newX, newY)
            ));
        }
    }

    m_undoStack->endMacro();
}

void ToolController::rotateSelected(qreal angleDelta)
{
    QVariantList indices = m_model->selectedIndices();
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
    if (index < 0 || index >= m_model->count()) return;
    
    ImageData item = m_model->getItem(index);
    if (item.pixmap.isNull()) return;
    
    qreal sourceWidth = (item.cropWidth > 0) ? item.cropWidth : item.pixmap.width();
    qreal sourceHeight = (item.cropHeight > 0) ? item.cropHeight : item.pixmap.height();
    
    qreal scaleX = sourceWidth / item.width;
    qreal scaleY = sourceHeight / item.height;
    
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
    QVariantList indices = m_model->selectedIndices();
    if (indices.isEmpty()) return;

    m_undoStack->beginMacro("Подписать изображения");

    for (const QVariant &v : indices) {
        int idx = v.toInt();
        ImageData item = m_model->getItem(idx);
        if (item.label != label) {
            m_undoStack->push(new SetLabelCommand(m_model, idx, item.label, label));
        }
    }

    m_undoStack->endMacro();
}

void ToolController::arrangeAll()
{
    int count = m_model->count();
    if (count == 0) return;

    int spacing = SettingsManager::instance().arrangeSpacing();

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
        ImageData data = m_model->getItem(i);
        
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

    qreal startX = 10000.0 - maxRowWidth / 2.0;
    
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
    qreal startY = 10000.0 - totalHeight / 2.0;

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

        ImageData data = m_model->getItem(item.index);
        sortedIndices.append(item.index);
        oldPositions.append(QPointF(data.x, data.y));
        newPositions.append(QPointF(newX, newY));

        currentX += item.bbWidth + spacing;
        rowHeight = std::max(rowHeight, item.bbHeight);
    }

    m_undoStack->push(new ArrangeCommand(m_model, sortedIndices, oldPositions, newPositions));
}
