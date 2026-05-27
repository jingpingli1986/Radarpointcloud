#ifndef PCAPEXTRACTSAVE_H
#define PCAPEXTRACTSAVE_H

#include <QObject>
#include <QFile>
#include <QVector>
#include <mutex>
#include "LatencyMonitor.h"

class ParserWorker : public QObject
{
    Q_OBJECT

public:
    ParserWorker();


    const QByteArray& getBigBuffer() const;

    const QVector<QByteArray>& getAllBuffers() const;

    // void ExtractRaw(const uchar* data, int totalLen, int payloadLen);
    void ExtractRaw(const uchar* data, int payloadLen);
    bool isFrameReady() const { return m_frameReady.load(); }

    // UI 取走数据后，调用这个重置状态
    void resetFrameReady() { m_frameReady.store(false); }

    // 供处理线程设置状态
    void setFrameReady(bool ready) { m_frameReady.store(ready); }

    // 获取最新帧的拷贝（加锁保护）
    QByteArray getLatestFrame() {
        std::lock_guard<std::mutex> lock(dataMutex);
        return latestCompleteFrame;
    }

    void updateLatestFrame() {
        std::lock_guard<std::mutex> lock(dataMutex);
        // 将拼接好的大数组同步到专门供外部读取的变量中
        latestCompleteFrame = bigBuffer;
    }
    void resetWriteOffset() {
        writeOffset = 0;
    }

    int getWriteOffset() const { return writeOffset; }

    bool isCollecting(){
        return m_isCollecting;
    }
    void setCollecting(bool isheader){
        m_isCollecting=isheader;
    }
    uint32_t lastFrameId() const { return m_lastFrameId; }

    // 设置当前帧 ID
    void setLastFrameId(uint32_t id) { m_lastFrameId = id; }

    // 在停止或启动时调用，重置 ID 状态
    void resetLastFrameId() { m_lastFrameId = 0; }
// signals:
//     void signalStartCapture();
//     void signalStopCapture();

    void clear() {
        m_lastFrameId = 0;
        writeOffset = 0;
        m_isCollecting = false;
        m_frameReady.store(false);
        // latestCompleteFrame.clear(); // 可选
    }

private:
    bool m_isCollecting =false;
    std::atomic<uint64_t> m_frameCount{0};     // 总接收帧数
    std::atomic<uint64_t> m_incompleteFrames{0}; // 异常帧数（丢包导致）
    bool m_hasHead = false; // 状态机：是否已收到帧头


    std::atomic<bool> m_frameReady{false}; // 原子布尔值

    QByteArray bigBuffer;

    QVector<QByteArray> bufferList;        // 已经完成的多个 bigBuffer
    int writeOffset = 0;

    std::mutex dataMutex;
    QByteArray latestCompleteFrame; // 专门存最新一帧

    // static constexpr int BUFFER_SIZE = 3330728;
    static constexpr int BUFFER_SIZE = 3330688;
    uint32_t m_lastFrameId = 0;

};

void processRadarFrameRealtime(const uchar* payload, int payloadLen, ParserWorker &worker);

#endif // PCAPEXTRACTSAVE_H
