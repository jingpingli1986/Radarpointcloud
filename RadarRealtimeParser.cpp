#include "RadarRealtimeParser.h"

RadarRealtimeParser::RadarRealtimeParser(QObject *parent)
    : QObject{parent}
{}

void RadarRealtimeParser::handleRawFrame(uint32_t frameId, const QByteArray &data) {
    qDebug() << "Parser: Start processing frame" << frameId << "Data size:" << data.size();

    // 1. 实时模式的偏移量（不含 EGO）
    const int HDR_OFFSET  = 0;
    const int EGO_OFFSET  = 72;
    const int MTN_OFFSET  = 136;
    const int OBJS_OFFSET = 188; // (72 + 64 + 52)

    if (data.size() < OBJS_OFFSET + sizeof(RadarDetObjList)) return;


    const char* ptr = data.constData();
    auto* fHdr = reinterpret_cast<const RadarFrameHeader*>(ptr + HDR_OFFSET);
    auto* ego = reinterpret_cast<const RadarEgoInfo*>(ptr + EGO_OFFSET);
    auto* mtn  = reinterpret_cast<const RadarMtnInfo*>(ptr + MTN_OFFSET);
    auto* objs = reinterpret_cast<const RadarDetObjList*>(ptr + OBJS_OFFSET);
    float currentSpeed = ego->egoSpeed;
    float currentYawRate = ego->egoYawRate;

    // float currentSpeed = static_cast<float>(std::rand() % 300);
    // float currentYawRate = static_cast<float>(std::rand() % 300);

    // qDebug()<<ego->timestampglobal;
    uint32_t count = objs->detObjNum;
    if (count > 2048) count = 2048; // 安全保护

    // 2. 创建点云对象
    pcl::PointCloud<PointXYZRGBWithProperties>::Ptr cloud(new pcl::PointCloud<PointXYZRGBWithProperties>);
    cloud->points.reserve(count);

    // 3. 快速解析
    for (uint32_t i = 0; i < count; ++i) {
        PointXYZRGBWithProperties p;
        // 坐标及补偿计算
        p.x = objs->xPos[i] ;
        p.y = -(objs->yPos[i]); //objs->yPos[i] ;
        p.z = objs->zPos[i];

        // p.x = static_cast<float>(std::rand() % 300);
        // p.y = static_cast<float>(QRandomGenerator::global()->bounded(300.0) - 150.0);
        // p.z = static_cast<float>(std::rand() % 300);

        // 填充自定义属性字段
        p.range        = objs->range[i];
        p.dopplerSpeed = objs->dopplerSpeed[i];
        p.SNRdB        = objs->snrdB[i];
        p.azimuthAng   = objs->azimuth[i];
        p.eleAng       = objs->elevation[i];
        p.rcsdB        = objs->rcsdB[i];
        p.Q_azi        = objs->aziQly[i];
        p.Q_ele        = objs->eleQly[i];
        p.detValid     =1;
        p.powerdB      = objs->powerdB[i];
        p.radVelAbs    = objs->radVelAbs[i];
        p.frameIndex   = frameId;
        p.originalIndex = i;

        //增加rangebin 和velbin 的属性显示
        p.rangebin      = objs->rangeBin[i];
        p.velbin        = objs->velBin[i];

        // 设置一个默认颜色（如白色）
        p.r = 255; p.g = 255; p.b = 255;
        cloud->points.push_back(p);
    }

    if (cloud && !cloud->points.empty()) {
        qDebug() << "Parser: Success! Cloud generated with" << cloud->points.size() << "points.";
        emit cloudReady(cloud, frameId,currentSpeed, currentYawRate);
    } else {
        qDebug() << "Parser: Error! Cloud is empty or null.";
    }

    qDebug() << "Cloud generated, points:" << cloud->points.size();

    // 4. 发射信号（直接交给 UI 线程的渲染器）
    // emit cloudReady(cloud, fHdr->frameId);
}
