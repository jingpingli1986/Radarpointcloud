// #ifndef LATENCYMONITOR_H
// #define LATENCYMONITOR_H

// #include <QMap>
// #include <QMutex>
// #include <QDateTime>
// #include <QDebug>

// class LatencyMonitor {
// public:
//     static LatencyMonitor& instance() {
//         static LatencyMonitor inst;
//         return inst;
//     }

//     // 记录特定帧在特定阶段的时间戳
//     void record(uint32_t frameId, const QString& stage) {
//         QMutexLocker locker(&m_mutex);
//         m_data[frameId][stage] = QDateTime::currentMSecsSinceEpoch();
//     }

//     // 汇总并打印结果
//     void report(uint32_t frameId) {
//         QMutexLocker locker(&m_mutex);
//         if (!m_data.contains(frameId)) return;

//         auto &stages = m_data[frameId];
//         qint64 t1 = stages["T1_Capture"];
//         qint64 t2 = stages["T2_Parsed"];
//         qint64 t3 = stages["T3_UI_Recv"];
//         qint64 t4 = stages["T4_Rendered"];

//         qDebug() << "------------------------------------------";
//         qDebug() << QString("Frame ID: %1 Latency Summary:").arg(frameId);
//         qDebug() << QString("1. Parsing (T2-T1):    %1 ms").arg(t2 - t1);
//         qDebug() << QString("2. Signal Lag (T3-T2): %1 ms (Thread Switching)").arg(t3 - t2);
//         qDebug() << QString("3. Rendering (T4-T3):  %1 ms (VTK Window Update)").arg(t4 - t3);
//         qDebug() << QString("== Total E2E Latency:  %1 ms ==").arg(t4 - t1);
//         qDebug() << "------------------------------------------";

//         m_data.remove(frameId); // 清理旧数据
//     }

// private:
//     LatencyMonitor() {}
//     QMutex m_mutex;
//     QMap<uint32_t, QMap<QString, qint64>> m_data;
// };


// #endif // LATENCYMONITOR_H

#ifndef LATENCYMONITOR_H
#define LATENCYMONITOR_H

#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

class LatencyMonitor : public QObject {
    Q_OBJECT
public:
    static LatencyMonitor& instance() {
        static LatencyMonitor inst;
        return inst;
    }

    void addFrameCount(uint32_t count = 1) {
        QMutexLocker locker(&m_mutex);
        m_totalFrames += count;
    }

    void addDroppedFrames(uint32_t count) {
        QMutexLocker locker(&m_mutex);
        m_droppedFrames += count;
        m_totalFrames += count; // 丢掉的帧也算在总帧数里

        // 计算丢帧率并触发信号
        double rate = (m_totalFrames == 0) ? 0.0 : (static_cast<double>(m_droppedFrames) / m_totalFrames) * 100.0;
        emit dropRateUpdated(rate, m_droppedFrames,m_totalFrames,m_overflowCount);
    }

    // 埋点记录函数
    void record(uint32_t frameId, const QString& stage) {
        QMutexLocker locker(&m_mutex);
        m_data[frameId][stage] = QDateTime::currentMSecsSinceEpoch();
    }

    // 汇总计算并触发信号
    void report(uint32_t frameId) {
        QMutexLocker locker(&m_mutex);
        // m_totalFrames++;
        if (!m_data.contains(frameId)) return;

        QMap<QString, qint64> &s = m_data[frameId];

        // 确保关键点都齐了
        if (s.contains("T1_Capture") && s.contains("T2_Parsed") &&
            s.contains("T3_UI_Recv") && s.contains("T4_Rendered"))
        {
            double parsing   = static_cast<double>(s["T2_Parsed"] - s["T1_Capture"]);
            double signalLag = static_cast<double>(s["T3_UI_Recv"] - s["T2_Parsed"]);
            double rendering = static_cast<double>(s["T4_Rendered"] - s["T3_UI_Recv"]);
            double total     = static_cast<double>(s["T4_Rendered"] - s["T1_Capture"]);

            // 控制台保留英文日志输出
            qDebug() << QString("[Frame %1] P:%2ms | S:%3ms | R:%4ms | Total:%5ms")
                            .arg(frameId).arg(parsing).arg(signalLag).arg(rendering).arg(total);

            emit newLatencyReport(parsing, signalLag, rendering, total);

            double rate = (m_totalFrames == 0) ? 0.0 : (static_cast<double>(m_droppedFrames) / m_totalFrames) * 100.0;
            emit dropRateUpdated(rate, m_droppedFrames,m_totalFrames,m_overflowCount);
        }

        m_data.remove(frameId);
        if (m_data.size() > 100) m_data.clear(); // 异常保护
    }

    void reset() {
        QMutexLocker locker(&m_mutex);
        m_data.clear();
        m_totalFrames = 0;
        m_droppedFrames = 0;
        m_overflowCount = 0;
        qDebug() << "LatencyMonitor cache cleared.";
    }
    void updateOverflowCount(uint64_t count) {
        QMutexLocker locker(&m_mutex);
        m_overflowCount = count;
        // 触发信号时，同时带上丢帧率和溢出数
        double rate = (m_totalFrames == 0) ? 0.0 : (static_cast<double>(m_droppedFrames) / m_totalFrames) * 100.0;
        emit dropRateUpdated(rate, m_droppedFrames, m_totalFrames, m_overflowCount);
    }

signals:
    void newLatencyReport(double p, double s, double r, double t);
    void dropRateUpdated(double rate, uint32_t totalDropped, uint32_t m_totalFrames, uint64_t overflow);
private:
    LatencyMonitor() : QObject(nullptr), m_totalFrames(0), m_droppedFrames(0) {}
    QMutex m_mutex;
    QMap<uint32_t, QMap<QString, qint64>> m_data;

    uint32_t m_totalFrames;    // 期望收到的总帧数
    uint32_t m_droppedFrames;  // 实际丢失的帧数
    uint64_t m_overflowCount = 0;
};

#endif
