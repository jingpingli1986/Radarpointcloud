#include "PacketProcessorThread.h"
#include <QDebug>
#include <QDateTime>

PacketProcessorThread::PacketProcessorThread(PacketQueue* queue,
                                             QObject* parent)
    : QThread(parent),
    packetQueue(queue)
{
}

const QVector<QByteArray>&PacketProcessorThread::getAllBuffers() const
{
    return radarWorker.getAllBuffers();
}


// 在 PacketProcessorThread.run() 中修改解析部分
void PacketProcessorThread::run() {
    FramePacket* packet = nullptr;
    while (packetQueue->pop(packet)) {
        const uchar* ptr = packet->frameData;
        int frameLen = packet->frameLen;

        uint16_t typeField = (ptr[12] << 8) | ptr[13];
        int offset = 14; // 默认偏移
        if (typeField == 0x8100) {
            offset = 18; // 发现 VLAN 标签，多跳 4 字节
        }


        // 基础长度校验
        if (frameLen < offset + 20 + 8) continue;

        // 1. IP 头
        const uchar* ipHeader = ptr + offset;
        int ipHeaderLen = (ipHeader[0] & 0x0F) * 4;

        // ✅ 建议三：校验 IP 头合法性
        if (ipHeaderLen < 20 || (offset + ipHeaderLen + 8) > frameLen) continue;
        if (ipHeader[9] != 17) continue; // 必须是 UDP

        // 2. UDP 头
        const uchar* udpHeader = ipHeader + ipHeaderLen;

        // ✅ 建议四：更严格的 UDP 长度校验
        uint16_t udpTotalLen = (udpHeader[4] << 8) | udpHeader[5];
        if (udpTotalLen < 8) continue;

        int payloadLen = udpTotalLen - 8;
        if (payloadLen <= 0 || (offset + ipHeaderLen + udpTotalLen) > frameLen) continue;

        const uchar* payload = udpHeader + 8;
        processRadarFrameRealtime(payload, payloadLen, radarWorker);
        if (radarWorker.isFrameReady()) {

            // 获取数据：假设 radarWorker 有 getLatestFrame() 或 getBuffer()
            // 如果返回的是 const QByteArray&，效率最高
            QByteArray fullFrame = radarWorker.getLatestFrame();

            // 既然没有 getCurrentFrameId，我们用本地计数
            uint32_t frameId = ++m_localFrameCount;

            // 这会增加 m_totalFrames，作为丢帧率的分母
            // LatencyMonitor::instance().addFrameCount(1);

            // 埋点 T1
            LatencyMonitor::instance().record(frameId, "T1_Capture");

            qDebug() << "!!! FRAME READY !!! 3.3MB assembled."; //
            if (m_parser) {
                m_parser->handleRawFrame(frameId, fullFrame);
            }

            // 发射信号给 RadarRealtimeParser
            // emit frameReady(frameId, fullFrame);

            // 埋点 T2
            LatencyMonitor::instance().record(frameId, "T2_Parsed");
            // 重置状态
            radarWorker.resetFrameReady();
        }

        // 别忘了释放 pop 出来的指针（取决于你的队列管理方式）
        // delete packet;
    }
}
