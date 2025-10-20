#include "PasteTool.h"     // ✅
#include "CanvasView.h"    // ✅
#include "ImageItem.h"     // ✅
#include "UndoRedoTool.h"  // ✅

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QImageReader>
#include <QUndoStack>
//#include <QViewport>

PasteTool::PasteTool(QObject* parent)
    : BaseTool(parent), m_supportedFormats(QImageReader::supportedImageFormats())
{
}

void PasteTool::execute()
{
    if (!m_canvasView || !m_undoStack) return;

    const QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    if (mimeData->hasUrls()) {
        bool imagePasted = false;
        for (const QUrl &url : mimeData->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QString extension = QFileInfo(filePath).suffix().toLower();
                if (m_supportedFormats.contains(extension.toUtf8())) {
                    QPixmap pixmap(filePath);
                    if (!pixmap.isNull()) {
                        ImageItem *imageItem = new ImageItem(pixmap, m_undoStack);
                        imageItem->setPos(m_canvasView->mapToScene(m_canvasView->viewport()->rect().center()));
                        m_undoStack->push(new AddCommand(imageItem, m_canvasView->scene()));
                        imagePasted = true;
                    }
                }
            }
        }
        if (imagePasted) {
            return;
        }
    }

    const QImage image = clipboard->image();
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        ImageItem *imageItem = new ImageItem(pixmap, m_undoStack);
        imageItem->setPos(m_canvasView->mapToScene(m_canvasView->viewport()->rect().center()));
        m_undoStack->push(new AddCommand(imageItem, m_canvasView->scene()));
    }
}
