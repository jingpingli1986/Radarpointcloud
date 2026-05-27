#ifndef PCAP_PROCESSOR_H
#define PCAP_PROCESSOR_H

#include <QObject>
#include <QString>
#include <QFile>
#include "radarreassembler.h"
#include <QDebug>
#include <atomic>


/**
 * @brief PcapProcessor 类负责 PCAP 文件的底层解析
 * 采用内存映射（Memory Mapping）技术以实现高性能读取
 */
class PcapProcessor : public QObject
{
    Q_OBJECT
public:
    explicit PcapProcessor(QObject *parent = nullptr);
    ~PcapProcessor();

    /**
     * @brief 执行 PCAP 文件解析
     * @param fileName PCAP 文件路径
     * @return 是否解析成功
     */
    bool run(const QString &fileName);

    void stop() { m_stop = true; } // 供外部调用
    bool isRunning() const { return m_working; }
public slots:

    void processBinFolder(const QString &folderPath);


signals:

    void binFolderLoadingFinished();
    /**
     * @brief 解析进度信号 (0-100)
     */
    void progressChanged(int percentage);

    /**
     * @brief 解析完成信号，转发重组后的数据
     * @param frameId 帧序号
     * @param data 完整的雷达帧原始数据（Payload）
     */
    void radarFrameParsed(quint32 frameId, const QByteArray &data);
    void binFrameParsed(quint32 frameId, const QByteArray &data);

private:
    // 禁止拷贝
    PcapProcessor(const PcapProcessor&) = delete;
    PcapProcessor& operator=(const PcapProcessor&) = delete;

    std::atomic<bool> m_stop{false};
    std::atomic<bool> m_working{false};
};

#endif // PCAP_PROCESSOR_H
