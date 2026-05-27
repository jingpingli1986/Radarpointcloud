#ifndef IMAGESAVER_H
#define IMAGESAVER_H

#include <QThread>
#include <QImage>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <atomic>

/**
 * @brief 异步图像保存器
 * 采用生产者-消费者模型，防止 JPG 编码和磁盘 IO 阻塞主 UI 线程或采集线程。
 */
class ImageSaver : public QThread {
    Q_OBJECT
public:
    const int MAX_QUEUE_SIZE = 100;

    struct SaveTask {
        QImage img;
        QString fullPath;
    };

    explicit ImageSaver(QObject *parent = nullptr)
        : QThread(parent), m_running(true) {}

    /**
     * @brief 停止线程并清空剩余队列
     */
    ~ImageSaver() {
        stop();
    }

    /**
     * @brief 添加保存任务
     * @param img 图像数据（内部自动处理内存独立性）
     * @param path 完整保存路径（含文件名）
     */
    void addTask(QImage img, const QString& path)
    {
        if (img.isNull() || path.isEmpty()) return;

        QMutexLocker locker(&m_mutex);
        if (m_queue.size() >= MAX_QUEUE_SIZE) return; // 内部保护

        if (!img.isDetached()) img = img.copy();
        m_queue.enqueue({std::move(img), path});
        m_condition.wakeOne();
    }

    /**
     * @brief 安全停止线程，确保数据写完
     */
    bool queueFull() {
        QMutexLocker locker(&m_mutex);
        return m_queue.size() >= MAX_QUEUE_SIZE;
    }
    int queueSize() {
        QMutexLocker locker(&m_mutex);
        return m_queue.size();
    }

    void stop()
    {
        {
            QMutexLocker locker(&m_mutex);
            if (!m_running) return;
            m_running = false;
        }

        m_condition.wakeAll();
        if (isRunning()) {
            wait(); // 等待 run() 循环结束
        }
    }


protected:
    void run() override
    {
        qDebug() << "ImageSaver thread started.";

        // 只要线程在运行，或者队列里还有没存完的图，就继续执行
        while (m_running || !m_queue.isEmpty())
        {
            SaveTask task;

            {
                QMutexLocker locker(&m_mutex);

                // 如果队列为空且还在运行，则进入休眠等待 wakeOne()
                while (m_queue.isEmpty() && m_running) {
                    m_condition.wait(&m_mutex);
                }

                // 如果被唤醒后发现要退出且队列已空，直接跳出
                if (m_queue.isEmpty() && !m_running)
                    break;

                task = m_queue.dequeue();
            }

            // --- 执行磁盘写入操作 (耗时操作) ---
            if (!task.img.isNull())
            {
                // 自动递归创建不存在的目录
                QFileInfo info(task.fullPath);
                info.absoluteDir().mkpath(".");

                // 保存为 JPG 格式，质量设为 80（工业平衡点）
                if (!task.img.save(task.fullPath, "JPG", 80)) {
                    qDebug() << "Critical: Image save failed!" << task.fullPath;
                }
            }
        }

        qDebug() << "ImageSaver thread stopped safely.";
    }

private:
    QQueue<SaveTask> m_queue;
    QMutex m_mutex;
    QWaitCondition m_condition;
    std::atomic<bool> m_running{true};
};

#endif // IMAGESAVER_H
