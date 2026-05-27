#ifndef PACKETPROCESSORTHREAD_H
#define PACKETPROCESSORTHREAD_H


#include <QThread>
#include "PacketQueue.h"
#include "pcapextractsave.h"
#include "RadarRealtimeParser.h"
#include "LatencyMonitor.h"
class PacketProcessorThread : public QThread
{
    Q_OBJECT
public:
    PacketProcessorThread(PacketQueue* queue,
                          QObject* parent = nullptr);

    const QVector<QByteArray>& getAllBuffers() const;
    ParserWorker* getWorker() { return &radarWorker; }

    void setParser(RadarRealtimeParser* p) { m_parser = p; }

signals:
    // --- 新增信号：拼好一帧原始数据后发射 ---
    void frameReady(uint32_t frameId, const QByteArray &data);



protected:
    void run() override;

private:

    RadarRealtimeParser* m_parser = nullptr;
    PacketQueue* packetQueue;
    ParserWorker radarWorker;
    uint32_t m_localFrameCount = 0; // 本地帧序号计数

};


#endif // PACKETPROCESSORTHREAD_H
