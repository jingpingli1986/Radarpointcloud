#include <QDebug>
#include <QDataStream>
#include <QtEndian>
#include <QByteArray>
#include "pcapextractsave.h"

ParserWorker::ParserWorker()
{
    bigBuffer.resize(BUFFER_SIZE);
    bigBuffer.fill(0);
}
const QByteArray& ParserWorker::getBigBuffer() const
{
    return bigBuffer;
}
const QVector<QByteArray>& ParserWorker::getAllBuffers() const
{
    return bufferList;
}
// 在 pcapextractsave.cpp 中修改
// void ParserWorker::ExtractRaw(const uchar* data, int /*totalLen*/, int payloadLen) {
//     int remain = BUFFER_SIZE - writeOffset;
//     int copySize = qMin(payloadLen, remain);

//     if (copySize > 0) {
//         memcpy(bigBuffer.data() + writeOffset, data, copySize);
//         writeOffset += copySize;
//     }

//     // ✅ 建议五：严谨的等于判断
//     if (writeOffset == BUFFER_SIZE) {
//         std::lock_guard<std::mutex> lock(dataMutex);
//         latestCompleteFrame = bigBuffer;
//         writeOffset = 0;
//         m_frameCount++;
//         m_hasHead = false; // 重置状态机，等待下一个帧头
//     }
// }

void ParserWorker::ExtractRaw(const uchar* data, int payloadLen) {
    if (writeOffset + payloadLen > BUFFER_SIZE) {
        payloadLen = BUFFER_SIZE - writeOffset;
    }

    if (payloadLen > 0) {
        memcpy(bigBuffer.data() + writeOffset, data, payloadLen);
        writeOffset += payloadLen;
    }
}
// void processRadarFrameRealtime(const QByteArray &udpData,
//                                ParserWorker &worker)
// {
//     // 1. 获取原始指针，避免 QByteArray 的封装开销
//     const uchar* rawPtr = reinterpret_cast<const uchar*>(udpData.constData());
//     int totalLen = udpData.size();

//     if (totalLen < 60) return;

//     // 2. 直接从指针读取 Magic Word (偏移量 12)
//     quint32 value = qFromLittleEndian<quint32>(rawPtr + 12);

//     if (value == 0x00BE0001) { // 帧头
//         worker.ExtractRaw(rawPtr, totalLen, 1288);
//     }
//     else if (value == 0x00000001) { // 帧内容
//         worker.ExtractRaw(rawPtr, totalLen, 1360);
//     }
//     else if (value == 0xEF000001) { // 帧尾
//         worker.ExtractRaw(rawPtr, totalLen, 48);
//         // 这里可以触发“最新帧已就绪”的逻辑
//         worker.setFrameReady(true);
//     }
// }
// 修改后的函数签名，接收原始指针
// 在 pcapextractsave.cpp 中hh
// void processRadarFrameRealtime(const uchar* payload, int payloadLen, ParserWorker &worker) {
//     if (payloadLen < 16) return;
//     quint32 value = qFromLittleEndian<quint32>(payload + 12);

//     if (value == 0x00BE0001) { // 帧头
//         // 如果上次帧没满就收到了新帧头，说明中间丢包了
//         // if (worker.getWriteOffset() != 0) worker.incIncompleteCount();
//         worker.ExtractRaw(payload, payloadLen, 1288);
//     }
//     else if (value == 0x00000001) { // 内容
//         worker.ExtractRaw(payload, payloadLen, 1360);
//     }
//     else if (value == 0xEF000001) { // 帧尾
//         worker.ExtractRaw(payload, payloadLen, 48);
//         worker.setFrameReady(true);
//     }
// }

void processRadarFrameRealtime(const uchar* payload, int payloadLen, ParserWorker &worker) {
    if (payloadLen < 40) return;

    quint32 magic = qFromLittleEndian<quint32>(payload + 12);
    quint32 currentFrameId = qFromLittleEndian<quint32>(payload + 16);

    const uchar* businessData = payload + 40;
    int businessLen = payloadLen - 40;

    if (magic == 0x00BE0001 || magic == 0xEFBE0001) {
        // --- 维度 1: 检查静默整帧丢失 ---
        if (worker.lastFrameId() != 0 && currentFrameId > worker.lastFrameId() + 1) {
            uint32_t missed = currentFrameId - worker.lastFrameId() - 1;
            LatencyMonitor::instance().addDroppedFrames(missed);
        }

        // --- 维度 2: 检查上一帧是否残缺 ---
        if (worker.isCollecting()) {
            LatencyMonitor::instance().addDroppedFrames(1);
        } else {
            // ✅ 关键修改：每看到一个正常的 Header 且当前没在收集，说明开启了新的一帧
            // 这里增加 totalFrames 保证分母正确
            LatencyMonitor::instance().addFrameCount(1);
        }
        worker.setLastFrameId(currentFrameId);
        worker.resetWriteOffset();
        worker.ExtractRaw(businessData, businessLen);
        worker.setCollecting(true);

    }
    else if (magic == 0x00000001) {
        if (worker.isCollecting()) {
            worker.ExtractRaw(businessData, businessLen);
        }
    }
    else if (magic == 0xEF000001) {
        if (worker.isCollecting()) {
            worker.ExtractRaw(businessData, businessLen);
            // 校验长度是否合理 (3.3MB 约等于 3300000 字节)
            if (worker.getWriteOffset() > 3000000) {
                worker.updateLatestFrame();
                worker.setFrameReady(true);
                worker.resetWriteOffset();
            }
            worker.setCollecting(false); // 完成或长度不对都重置
    }   }
}
