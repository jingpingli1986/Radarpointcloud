// PacketRecordWorker.h
#include <QObject>
#include <QByteArray>
#include "pcap.h"
#include <QDebug>

class PacketRecordWorker : public QObject {
    Q_OBJECT
public:
    explicit PacketRecordWorker(QObject *parent = nullptr) : QObject(parent) {}
    ~PacketRecordWorker() { closeFile(); }

public slots:
    // 初始化录制，传入 pcap 句柄用于创建 dumper
    void onStartRecording(const QString &path) {
        if (m_pcapDumper) onStopRecording();

        // 关键修复：不从外部传 handle，自己创建一个虚拟句柄来打开 dumper
        // 链路类型 DLT_EN10MB (以太网)，捕获长度 65535
        pcap_t* tempHandle = pcap_open_dead(DLT_EN10MB, 65535);
        if (tempHandle) {
            m_pcapDumper = pcap_dump_open(tempHandle, path.toStdString().c_str());
            pcap_close(tempHandle); // 打开 dumper 后，tempHandle 即可关闭
        }

        if (m_pcapDumper) {
            qDebug() << "PCAP Dumper started saving to:" << path;
        } else {
            qDebug() << "PCAP Dumper Error: Cannot open file for writing!";
        }
    }

    // 真正的写盘操作，运行在录制线程
    void onPacketCaptured(const QByteArray &data, pcap_pkthdr header) {
        if (m_pcapDumper) {
            // pcap_dump 即使阻塞，也只会阻塞当前的 Record 线程，不会影响抓包线程
            pcap_dump((u_char*)m_pcapDumper, &header, (const u_char*)data.constData());
        }
    }

    void onStopRecording() {
        closeFile();
    }

private:
    void closeFile() {
        if (m_pcapDumper) {
            pcap_dump_flush(m_pcapDumper); // 确保缓冲区刷入磁盘
            pcap_dump_close(m_pcapDumper);
            m_pcapDumper = nullptr;
            qDebug() << "Worker: PCAP saved and closed.";
        }
    }

    pcap_dumper_t* m_pcapDumper = nullptr;
};
