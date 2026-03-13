#include "UpscaleController.h"
#include "ImageModel.h"
#include "ModelsManager.h"
#include "StackController.h" // Added for QUndoCommand

#include <QThreadPool>
#include <QPainter>
#include <QDebug>
#include "net.h"
#include "cpu.h"
#include "mat.h"

UpscaleWorker::UpscaleWorker(int index, const QImage &image, const QString &modelPath, const QString &paramPath)
    : m_index(index), m_image(image), m_modelPath(modelPath), m_paramPath(paramPath) {
    setAutoDelete(true);
}

void UpscaleWorker::run() {
    QImage srcImage = m_image;
    if (srcImage.isNull()) {
        emit failed(m_index, "Invalid image data");
        return;
    }

    // Convert to RGBA8888 for consistent processing
    srcImage = srcImage.convertToFormat(QImage::Format_RGBA8888);
    int w = srcImage.width();
    int h = srcImage.height();

    try {
        ncnn::Net net;
        // Optimization options
        net.opt.use_vulkan_compute = false; // CPU fallback by default, or auto if Vulkan is enabled in ncnn
        net.opt.use_fp16_packed = true;
        net.opt.use_fp16_storage = true;
        net.opt.use_fp16_arithmetic = true;
        
        int r1 = net.load_param(m_paramPath.toUtf8().constData());
        int r2 = net.load_model(m_modelPath.toUtf8().constData());
        
        if (r1 != 0 || r2 != 0) {
            emit failed(m_index, "Failed to load ncnn model");
            return;
        }

        // Convert into ncnn matrix (discarding alpha for the network, format RGBA2RGB)
        ncnn::Mat in = ncnn::Mat::from_pixels(srcImage.constBits(), ncnn::Mat::PIXEL_RGBA2RGB, w, h);

        // Models typically expect [0, 1] floats. ncnn from_pixels gives [0, 255].
        const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
        in.substract_mean_normalize(0, norm_vals);

        ncnn::Extractor ex = net.create_extractor();
        ex.set_light_mode(true);
        
        ex.input("data", in);
        
        ncnn::Mat out;
        ex.extract("output", out);
        
        if (out.empty()) {
            emit failed(m_index, "Inference output is empty");
            return;
        }
        
        // Scale back to [0, 255] if the model outputs [0, 1]
        const float denorm_vals[3] = {255.f, 255.f, 255.f};
        out.substract_mean_normalize(0, denorm_vals);
        
        // Reconstruct image with scaled alpha if necessary
        QImage scaledImage(out.w, out.h, QImage::Format_RGBA8888);
        out.to_pixels(scaledImage.bits(), ncnn::Mat::PIXEL_RGB2RGBA);
        
        // If original image had transparency, upscale alpha with bicubic and reapply
        if (srcImage.hasAlphaChannel()) {
            // Исходник у нас в RGBA8888. Мы можем достать альфа-канал напрямую:
            QImage alphaOnly(w, h, QImage::Format_Grayscale8);
            for (int y = 0; y < h; ++y) {
                const uchar *srcRow = srcImage.constScanLine(y);
                uchar *alphaRow = alphaOnly.scanLine(y);
                for (int x = 0; x < w; ++x) {
                    alphaRow[x] = srcRow[x * 4 + 3]; // Alpha is the 4th byte
                }
            }
            
            QImage scaledAlpha = alphaOnly.scaled(out.w, out.h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            
            // Reapply alpha to the scaled rgba8888 (memory layout: R, G, B, A in bytes)
            for (int y = 0; y < out.h; ++y) {
                uchar *rgbaRow = scaledImage.scanLine(y);
                const uchar *alphaRow = scaledAlpha.constScanLine(y);
                for (int x = 0; x < out.w; ++x) {
                    rgbaRow[x * 4 + 3] = alphaRow[x]; // Set 4th byte (Alpha)
                }
            }
        }
        
        emit finished(m_index, scaledImage);
        
    } catch (const std::exception &e) {
        emit failed(m_index, QString("Exception during upscale: %1").arg(e.what()));
    }
}

UpscaleController::UpscaleController(ImagoImageModel *model, ModelsManager *modelsManager, QUndoStack *undoStack, QObject *parent)
    : QObject(parent), m_model(model), m_modelsManager(modelsManager), m_undoStack(undoStack) {
}

void UpscaleController::upscaleImage(int index) {
    if (index < 0 || index >= m_model->getCount()) return;
    if (m_activeTasks.contains(index)) return; // Already upscaling this image
    
    if (!m_modelsManager->isModelDownloaded()) {
        emit upscaleFailed(index, "Upscale model is not downloaded");
        return;
    }

    ImagoImageData data = m_model->getItem(index);
    if (data.pixmap.isNull()) {
        emit upscaleFailed(index, "Empty image");
        return;
    }

    m_activeTasks.insert(index);
    emit upscaleStarted(index);

    QImage srcImage = data.pixmap.toImage();
    
    // To save processing time and physically preserve the crop, extract it BEFORE upscaling
    if (data.cropWidth > 0 && data.cropHeight > 0) {
        srcImage = srcImage.copy(data.cropX, data.cropY, data.cropWidth, data.cropHeight);
    }
    
    UpscaleWorker *worker = new UpscaleWorker(index, srcImage, m_modelsManager->getModelPath(), m_modelsManager->getParamPath());
    connect(worker, &UpscaleWorker::finished, this, &UpscaleController::onUpscaleFinished, Qt::QueuedConnection);
    connect(worker, &UpscaleWorker::failed, this, &UpscaleController::onUpscaleFailed, Qt::QueuedConnection);

    QThreadPool::globalInstance()->start(worker);
}

void UpscaleController::onUpscaleFinished(int index, QImage result) {
    m_activeTasks.remove(index);
    if (index >= 0 && index < m_model->getCount()) {
        ImagoImageData data = m_model->getItem(index);
        
        QPixmap oldPixmap = data.pixmap;
        QRectF oldCrop(data.cropX, data.cropY, data.cropWidth, data.cropHeight);
        QPixmap newPixmap = QPixmap::fromImage(result);
        QRectF newCrop(0, 0, 0, 0); // No crop initially for the upscaled version
        
        // Push to undo stack
        if (m_undoStack) {
            m_undoStack->push(new UpscaleImageCommand(
                m_model, index,
                oldPixmap, oldCrop,
                newPixmap, newCrop
            ));
        } else {
            // Fallback (just in case)
            m_model->setPixmap(index, newPixmap);
            if (data.cropWidth > 0 && data.cropHeight > 0) {
                m_model->setCrop(index, 0, 0, 0, 0);
            }
        }
    }
    emit upscaleFinished(index);
}

void UpscaleController::onUpscaleFailed(int index, QString error) {
    m_activeTasks.remove(index);
    qWarning() << "Upscale failed for index" << index << ":" << error;
    emit upscaleFinished(index);
}
