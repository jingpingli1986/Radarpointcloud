// #ifndef VIDEOCLASS_H
// #define VIDEOCLASS_H
// #include <QMediaPlayer>
// #include <QAbstractVideoSurface>
// #include <QVideoFrame>
// #include <QVideoSurfaceFormat>
// #include <QLabel>
// #include <QMatrix> // 导入用于转换的类

// class VideoSurface : public QAbstractVideoSurface {
//     Q_OBJECT
// public:
//     explicit VideoSurface(QLabel* label, QObject* parent = nullptr)
//         : QAbstractVideoSurface(parent), m_label(label), m_rotation(0) {}

//     // 🚀 新增设置旋转的方法
//     void setRotation(int angle) { m_rotation = angle; }

//     QList<QVideoFrame::PixelFormat> supportedPixelFormats(
//         QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override {
//         Q_UNUSED(handleType);
//         // 为了兼容性，建议增加兼容格式
//         return {
//             QVideoFrame::Format_RGB32,
//             QVideoFrame::Format_ARGB32,
//             QVideoFrame::Format_ARGB32_Premultiplied,
//             QVideoFrame::Format_RGB24 // 增加常见格式
//         };
//     }

//     bool present(const QVideoFrame &frame) override {
//         if (!frame.isValid())
//             return false;

//         QVideoFrame copy(frame);
//         if (!copy.map(QAbstractVideoBuffer::ReadOnly))
//             return false;

//         QImage img(
//             copy.bits(),
//             copy.width(),
//             copy.height(),
//             copy.bytesPerLine(),
//             QVideoFrame::imageFormatFromPixelFormat(copy.pixelFormat())
//             );

//         // 深拷贝一份，因为 copy 对象很快会被 unmap
//         QImage finalImg = img.copy();
//         copy.unmap();

//         if (!finalImg.isNull()) {
//             // 🚀 执行旋转逻辑
//             if (m_rotation != 0) {
//                 QMatrix matrix;
//                 matrix.rotate(m_rotation);
//                 finalImg = finalImg.transformed(matrix);
//             }

//             // 更新到 Label
//             m_label->setPixmap(
//                 QPixmap::fromImage(finalImg)
//                     .scaled(m_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
//                 );
//         }

//         return true;
//     }

// private:
//     QLabel* m_label;
//     int m_rotation; // 🚀 存储角度：0, 90, 180, 270
// };
// #endif // VIDEOCLASS_H


#ifndef VIDEOCLASS_H
#define VIDEOCLASS_H

#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QLabel>
#include <QMatrix>
#include <QDateTime>
#include <QPointer>
#include <atomic>
#include <QMutex>
#include "ImageSaver.h"

class VideoSurface : public QAbstractVideoSurface {
    Q_OBJECT
public:
    explicit VideoSurface(QLabel* label, QObject* parent = nullptr)
        : QAbstractVideoSurface(parent), m_label(label), m_rotation(0) {
        m_imageSaver = new ImageSaver(this);
        m_imageSaver->start();
    }

    ~VideoSurface() override {
        if (m_imageSaver) m_imageSaver->stop();
    }

    void setRotation(int angle) { m_rotation = angle; }

    void setRecording(bool enabled, const QString& dir = "") {
        QMutexLocker locker(&m_mutex);
        m_recordDir = dir;
        m_isRecording = enabled;
        m_frameId = 0;
        m_droppedFrames = 0;
    }

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override {
        return {
            QVideoFrame::Format_RGB32,
            QVideoFrame::Format_ARGB32,
            QVideoFrame::Format_ARGB32_Premultiplied,
            QVideoFrame::Format_RGB24 // 增加常见格式
        };
    }

    bool present(const QVideoFrame &frame) {
        if (!frame.isValid()) return false;

        QVideoFrame mappedFrame(frame);
        if (!mappedFrame.map(QAbstractVideoBuffer::ReadOnly)) return false;

        // 1. 构造原始 QImage
        QImage rawImg(mappedFrame.bits(), mappedFrame.width(), mappedFrame.height(),
                      mappedFrame.bytesPerLine(),
                      QVideoFrame::imageFormatFromPixelFormat(mappedFrame.pixelFormat()));

        // 2. 立即深拷贝！脱离对 mappedFrame 内存的依赖
        // 💡 这一步非常重要，确保我们处理的是独立的内存数据
        QImage processedImg = rawImg.copy();
        mappedFrame.unmap(); // 安全释放硬件映射内存

        if (processedImg.isNull()) return false;

        // ============================================================
        // --- 🚀 核心修改：图像几何变换 (为了实现“所见即所得”保存) ---
        // ============================================================

        // 1. 镜像处理 (如果开启)
        if (m_mirrored) {
            processedImg = processedImg.mirrored(true, false);
        }

        // 2. 旋转处理 (如果开启)
        // 💡 关键：在保存之前执行旋转，直接修改 processedImg
        if (m_rotation != 0) {
            QMatrix matrix;
            matrix.rotate(m_rotation);
            // transformed 会返回一个新的 QImage，需要重新赋值
            processedImg = processedImg.transformed(matrix);
        }

        // ============================================================
        // --- 异步保存 (此时 processedImg 已经是【已镜像 + 已旋转】的图) ---
        // ============================================================
        QString savePath;
        {
            QMutexLocker locker(&m_mutex);
            if (m_isRecording) savePath = m_recordDir;
        }

        if (!savePath.isEmpty()) {
            if (m_imageSaver && !m_imageSaver->queueFull()) {
                qint64 ts = QDateTime::currentMSecsSinceEpoch();
                uint64_t id = m_frameId.fetch_add(1);
                QString fileName = QString("%1/%2_%3.jpg").arg(savePath).arg(ts).arg(id, 4, 10, QChar('0'));

                // 💡 这里发送给 ImageSaver 的就是最终效果图
                m_imageSaver->addTask(processedImg, fileName);
            } else {
                m_droppedFrames++;
            }
        }

        // ============================================================
        // --- 显示逻辑 ---
        // ============================================================
        // 此时 processedImg 已经是旋转过的了，直接拿来做缩放显示即可
        QImage displayImg = processedImg;

        QSize targetSize = m_label ? m_label->size() : QSize();
        if (!targetSize.isEmpty()) {
            // 这一步缩放只影响 UI 显示，不影响保存的图片分辨率
            displayImg = displayImg.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (m_label) {
            QMetaObject::invokeMethod(m_label, [this, displayImg]() {
                if (m_label) m_label->setPixmap(QPixmap::fromImage(displayImg));
            }, Qt::QueuedConnection);
        }

        return true;
    }

    uint64_t droppedFrames() const { return m_droppedFrames; }

    void setMirrored(bool mir) { m_mirrored = mir; }

    // 在 private 部分添加

private:
    QPointer<QLabel> m_label;
    int m_rotation;

    ImageSaver* m_imageSaver = nullptr;
    std::atomic<bool> m_isRecording{false};
    QString m_recordDir;
    std::atomic<uint64_t> m_frameId{0};
    std::atomic<uint64_t> m_droppedFrames{0};
    QMutex m_mutex;
    bool m_mirrored = false; // 默认为 true，因为大部分 USB 摄像头原始流是镜像的
};

#endif // VIDEOCLASS_H
