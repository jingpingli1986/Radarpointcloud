#ifndef RADAR_DATAPARSER_H
#define RADAR_DATAPARSER_H

#include <QObject>
#include <QByteArray>
#include "RadarProtocol.h" // 包含你定义的那些结构体
#include "rspprocess.h"


class RadarDataParser : public QObject {
    Q_OBJECT
public:
    explicit RadarDataParser(QObject *parent = nullptr);
    void clear();
    const std::vector<pointandtimedata8x8>& getFrames() const ;
    mutable QReadWriteLock m_lock; // 使用读写锁性能更好
    pointandtimedata8x8 RadarDataParser::getFrameAt(int index) const {
        QReadLocker locker(&m_lock);
        if (index >= 0 && index < (int)m_frames.size()) {
            return m_frames[index];
        }
        return {};
    }
    void parseFrame(quint32 frameId, const QByteArray &data, bool isRawBin);//16TR BIN

public slots:
    /**
     * @brief 核心解析槽函数
     * 接收来自 PcapProcessor 的原始帧数据
     */
    void parseFrame(quint32 frameId, const QByteArray &data); // 增加参数

signals:
    /**
     * @brief 解析完成信号
     * 可以发送处理后的点云列表或自车状态
     */
    void frameReady(int index); // 发送当前存储在 m_frames 中的索引
    void pointsProcessed(const QVector<float> &x, const QVector<float> &y);

private:
    std::vector<pointandtimedata8x8> m_frames;
    qint64 m_successFrameCount = 0;
    qint64 m_baseTimeNs = 0;
};

#endif
