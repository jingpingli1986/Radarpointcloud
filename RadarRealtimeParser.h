#ifndef RADAR_REALTIME_PARSER_H
#define RADAR_REALTIME_PARSER_H

#include <QObject>
#include <QByteArray>
#include "RadarProtocol.h" // 包含结构体定义
#include "rspprocess.h"    // 包含你的 PCL 定义

class RadarRealtimeParser : public QObject {
    Q_OBJECT
public:
    explicit RadarRealtimeParser(QObject *parent = nullptr);

signals:
    // 直接发送 PCL 指针，这是实时显示最快的方式（智能指针引用计数，不拷贝数据）
    void cloudReady(pcl::PointCloud<PointXYZRGBWithProperties>::Ptr cloud, uint32_t frameId,float speed,float yawRate);

public slots:
    // 接收来自 PacketProcessorThread 的信号
    void handleRawFrame(uint32_t frameId, const QByteArray &data);
};

#endif
