#ifndef RADAR_CAPTURE_MANAGER_H
#define RADAR_CAPTURE_MANAGER_H

#include <QObject>
#include "PacketQueue.h"
#include "PacketCaptureThread.h"
#include "PacketProcessorThread.h"
#include "RadarRealtimeParser.h"


class RadarCaptureManager : public QObject {
    Q_OBJECT
public:
    explicit RadarCaptureManager(QObject *parent = nullptr);
    ~RadarCaptureManager();

    // 启动接口
    void start(const QString &device, const QString &ip, int port);
    void stop();
    void setRecording(bool enabled, const QString &pcapPath = "") {
        if (m_capThread) {
            m_capThread->setRecording(enabled, pcapPath);
        }
    }

signals:
    // 转发解析器的信号给 UI
    void cloudReady(pcl::PointCloud<PointXYZRGBWithProperties>::Ptr cloud, uint32_t frameId,float speed,float yawRate);

private:
    PacketQueue* m_queue = nullptr;
    PacketCaptureThread* m_capThread = nullptr;
    PacketProcessorThread* m_procThread = nullptr;
    RadarRealtimeParser* m_parser = nullptr;
};

#endif
