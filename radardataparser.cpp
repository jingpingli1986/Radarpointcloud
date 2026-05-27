#include "radardataparser.h"
#include <QtEndian>
#include <QDebug>
#include <immintrin.h>


RadarDataParser::RadarDataParser(QObject *parent) : QObject(parent) {}

void RadarDataParser::parseFrame(quint32 frameId, const QByteArray &data) {
    if (data.size() < (188 + sizeof(RadarDetObjList) + sizeof(RadarParaInfo))) return;
    if (data.size()!=3330688) return;
    qDebug() << "Parser: Start processing frame" << frameId << "Data size:" << data.size();

    // 保存每一帧为bin 3330616
    QString timeFolder = QDateTime::currentDateTime().toString("yyyyMMdd_hh");
    QString basePath = QCoreApplication::applicationDirPath();
    QString folderPath = basePath + "/radar_data/" + timeFolder;
    QDir dir;
    if (!dir.exists(folderPath)) {
        dir.mkpath(folderPath);
    }

    // 5️⃣ 截取数据
    QByteArray binData = data.mid(72);

    QString fileName = QString("%1/frame_%2.bin")
                           .arg(folderPath)
                           .arg(frameId);

    // 7️⃣ 写文件
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file:" << fileName;
        return;
    }

    file.write(binData);
    file.close();


    const char* ptr = data.constData();

    // 1. 绝对物理偏移（基于你的 72+3330616 结构）
    const int HDR_OFFSET  = 0;
    const int EGO_OFFSET  = 72;
    const int MTN_OFFSET  = 72 + 64;      // 136
    const int OBJS_OFFSET = 72 + 64 + 52; // 188
    const int PARA_OFFSET = 188 + sizeof(RadarDetObjList);
    const int RDMAP_OFFSET = PARA_OFFSET + sizeof(RadarParaInfo);

    if (data.size() < (RDMAP_OFFSET + sizeof(RadarRDMap))) return;

    // 2. 映射结构体
    auto* fHdr = reinterpret_cast<const RadarFrameHeader*>(ptr + HDR_OFFSET);
    auto* ego = reinterpret_cast<const RadarEgoInfo*>(ptr + EGO_OFFSET);
    auto* mtn  = reinterpret_cast<const RadarMtnInfo*>(ptr + MTN_OFFSET);
    auto* objs = reinterpret_cast<const RadarDetObjList*>(ptr + OBJS_OFFSET);
    auto* rdMapData = reinterpret_cast<const RadarRDMap*>(ptr + RDMAP_OFFSET);

    // 3. 直接读取（小端环境不需要转换）
    uint32_t realId = fHdr->frameId;
    uint32_t count  = objs->detObjNum;

    // static qint64 baseTimeNs = 1775714296937LL * 1000000LL;

    // 显示每一帧的全球时间
    // qint64 framegolbalTime = ego->timestampglobal;

    // qDebug() << "Global Time:" << framegolbalTime;

    qint64 currentFrameNs = static_cast<qint64>(ego->timestampglobal) * 1000LL;

    pointandtimedata8x8 frameRecord;
    // frameRecord.indexno = realId;

    // frameRecord.start_time_ns = baseTimeNs;

    frameRecord.egoSpeed = ego->egoSpeed;
    frameRecord.egoYawRate = ego->egoYawRate;
    // frameRecord.egoSpeed =     static_cast<float>(std::rand() % 300);

    // frameRecord.egoYawRate =     static_cast<float>(std::rand() % 300);


    // frameRecord.timestamp_ns = fHdr->timestampNanoseconds; // 这里建议根据实际协议字段修改
    // frameRecord.start_time_ns = 0;

    frameRecord.Radar8T8Point.cloud.reset(new pcl::PointCloud<PointXYZRGBWithProperties>);

    frameRecord.Radar8T8Point.cloud->points.reserve(count);


    // 4. 循环解析点云
    for (uint32_t i = 0; i < count; ++i) {
        PointXYZRGBWithProperties p;

        // 坐标及补偿计算
        p.x = objs->xPos[i] ;
        p.y = -(objs->yPos[i]);//objs->yPos[i] ;
        p.z = objs->zPos[i];

        // 填充自定义属性字段
        p.range        = objs->range[i];
        p.dopplerSpeed = objs->dopplerSpeed[i];
        p.SNRdB        = objs->snrdB[i];
        p.azimuthAng   = objs->azimuth[i];
        p.eleAng       = objs->elevation[i];
        p.rcsdB        = objs->rcsdB[i];
        p.Q_azi        = objs->aziQly[i];
        p.Q_ele        = objs->eleQly[i];
        p.detValid     = 1;
        p.powerdB      = objs->powerdB[i];
        p.radVelAbs    = objs->radVelAbs[i];
        p.frameIndex   = m_successFrameCount;
        p.originalIndex = i;
        //增加rangebin 和velbin 的属性显示
        p.rangebin      = objs->rangeBin[i];
        p.velbin        = objs->velBin[i];

        // 设置一个默认颜色（如白色）
        p.r = 255; p.g = 255; p.b = 255;

        frameRecord.Radar8T8Point.cloud->points.push_back(p);
    }

    frameRecord.Radar8T8Point.cloud->width = frameRecord.Radar8T8Point.cloud->points.size();
    frameRecord.Radar8T8Point.cloud->height = 1;
    frameRecord.Radar8T8Point.cloud->is_dense = true;

    // frameRecord.Radar8T8Point.rdmap.resize(524288);
    // for (int i = 0; i < 524288; ++i) {
    //     frameRecord.Radar8T8Point.rdmap[i] = static_cast<float>(rdMapData->rdData[i]);
    // }

    // 6. 【关键操作】：将解析好的帧加入到采集容器 m_frames 中
    int index;
    {
        QWriteLocker locker(&m_lock);

        if (m_successFrameCount == 0) {
            m_baseTimeNs = currentFrameNs;
        }

        // qint64 relativeOffsetNs = m_successFrameCount * 50000000LL;
        frameRecord.start_time_ns = m_baseTimeNs;
        // timestamp_ns 记录当前帧距离开始时间过去了多久
        frameRecord.timestamp_ns = currentFrameNs - m_baseTimeNs;

        frameRecord.indexno=m_successFrameCount;
        m_frames.push_back(std::move(frameRecord));
        index = m_frames.size() - 1;
    }

    // m_frames.push_back(std::move(frameRecord));

    m_successFrameCount++;
    emit frameReady(static_cast<int>(index));

}

void RadarDataParser::clear() {
    // 释放 vector 内存并清空元素
    m_frames.clear();
    m_successFrameCount = 0;
    m_baseTimeNs = 0;
    // 可选：如果想强制释放内存空间（不仅仅是清空元素）
    // std::vector<pointandtimedata8x8>().swap(m_frames);
}
const std::vector<pointandtimedata8x8>& RadarDataParser::getFrames() const {
    return m_frames;
}

// 专为【完整AVI + 中间段BIN】设计的解析方法
void RadarDataParser::parseFrame(quint32 frameId, const QByteArray &data, bool isRawBin) {
    Q_UNUSED(isRawBin);

    const int EXPECTED_BIN_SIZE = 3330616;
    if (data.size() < (116 + sizeof(RadarDetObjList) + sizeof(RadarParaInfo))) return;
    if (data.size() != EXPECTED_BIN_SIZE) return;

    qDebug() << "Parser [Sync Mode]: Processing frameId:" << frameId
             << "Total logic index in memory:" << m_successFrameCount;

    const char* ptr = data.constData();

    // 1. 绝对物理偏移
    const int EGO_OFFSET   = 0;
    const int MTN_OFFSET   = 64;
    const int OBJS_OFFSET  = 64 + 52;
    const int PARA_OFFSET  = 116 + sizeof(RadarDetObjList);
    const int RDMAP_OFFSET = PARA_OFFSET + sizeof(RadarParaInfo);

    if (data.size() < (RDMAP_OFFSET + sizeof(RadarRDMap))) return;

    // 2. 映射结构体
    auto* ego  = reinterpret_cast<const RadarEgoInfo*>(ptr + EGO_OFFSET);
    auto* objs = reinterpret_cast<const RadarDetObjList*>(ptr + OBJS_OFFSET);

    uint32_t count = objs->detObjNum;

    // 3. 初始化点云记录
    pointandtimedata8x8 frameRecord;
    frameRecord.egoSpeed = ego->egoSpeed;
    frameRecord.egoYawRate = ego->egoYawRate;

    frameRecord.Radar8T8Point.cloud.reset(new pcl::PointCloud<PointXYZRGBWithProperties>);
    frameRecord.Radar8T8Point.cloud->points.reserve(count);

    // 4. 循环解析点云
    for (uint32_t i = 0; i < count; ++i) {
        PointXYZRGBWithProperties p;
        p.x = objs->xPos[i];
        p.y = -(objs->yPos[i]);
        p.z = objs->zPos[i];
        p.range        = objs->range[i];
        p.dopplerSpeed = objs->dopplerSpeed[i];
        p.SNRdB        = objs->snrdB[i];
        p.azimuthAng   = objs->azimuth[i];
        p.eleAng       = objs->elevation[i];
        p.rcsdB        = objs->rcsdB[i];
        p.Q_azi        = objs->aziQly[i];
        p.Q_ele        = objs->eleQly[i];
        p.detValid     = 1;
        p.powerdB      = objs->powerdB[i];
        p.radVelAbs    = objs->radVelAbs[i];
        p.frameIndex   = frameId; // 追溯真实硬件帧号
        p.originalIndex = i;
        p.rangebin      = objs->rangeBin[i];
        p.velbin        = objs->velBin[i];
        p.r = 255; p.g = 255; p.b = 255;

        frameRecord.Radar8T8Point.cloud->points.push_back(p);
    }

    frameRecord.Radar8T8Point.cloud->width = frameRecord.Radar8T8Point.cloud->points.size();
    frameRecord.Radar8T8Point.cloud->height = 1;
    frameRecord.Radar8T8Point.cloud->is_dense = true;

    // 5. 【关键互斥锁】：直接采用硬件 frameId 映射绝对时间轴，完美对齐完整 AVI 视频
    int index;
    {
        QWriteLocker locker(&m_lock);

        frameRecord.start_time_ns = 0;
        // 这里的真实硬件帧号是 frameId (1 ~ 8)
        frameRecord.timestamp_ns = static_cast<qint64>(frameId) * 50000000LL;

        frameRecord.indexno = frameId;


        // 🌟 核心修复点 1：防止越界，动态将容器大小扩容到足够容纳该 frameId
        if (frameId >= static_cast<quint32>(m_frames.size())) {
            m_frames.resize(frameId + 1);
        }

        // 🌟 核心修复点 2：直接精准存放在硬件帧号对应的位置上！不使用 push_back
        m_frames[frameId] = std::move(frameRecord);

        // 传递给 MainWindow 的 index 也就是真实的 frameId
        index = static_cast<int>(frameId);
    }

    m_successFrameCount++;

    // 6. 触发原有信号，上层 UI 刷新
    emit frameReady(static_cast<int>(index));
}
