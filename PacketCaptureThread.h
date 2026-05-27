#ifndef PACKETCAPTURETHREAD_H
#define PACKETCAPTURETHREAD_H

#include <QThread>
#include <QString>
#include "PacketQueue.h"
#include "pcap.h"

class PacketCaptureThread : public QThread
{
    Q_OBJECT
public:
    PacketCaptureThread(QString devName,
                        QString dstIp,
                        quint16 dstPort,
                        // int frameLen,
                        PacketQueue* queue,
                        QObject* parent = nullptr);
    ~PacketCaptureThread() {
        stopCapture();
        wait(); // 确保线程完全退出
        if (m_pcapDumper) {
            pcap_dump_close(m_pcapDumper);
        }
    }

    uint64_t getTotalCaptured() const { return totalCaptured.load(); }
    uint64_t getKernelDrop() const { return kernelDrop.load(); }

    void setRecording(bool enabled, QString path = "") {
        if (enabled) {
            m_savePath = path;
            m_isRecording = true;
        } else {
            m_isRecording = false;
            // 关闭逻辑在 packetHandler 内部由 next packet 触发，或在 run 末尾强制触发
        }
    }

protected:
    void run() override;

private:
    static void packetHandler(u_char* param,
                              const struct pcap_pkthdr* header,
                              const u_char* pkt_data);

    QString deviceName;
    QString filterIp;
    quint16 filterPort;
    int filterFrameLen;
    PacketQueue* packetQueue;

    pcap_t* handle = nullptr;
    std::atomic<bool> running;

    std::atomic<uint64_t> totalCaptured {0};
    std::atomic<uint64_t> kernelDrop {0};

    pcap_dumper_t* m_pcapDumper = nullptr; // PCAP 写入句柄
    std::atomic<bool> m_isRecording{false}; // 原子布尔值，保证多线程安全
    QString m_savePath;                     // 文件保存绝对路径

public slots:
    // void startCapture();
    void stopCapture();
};

#endif // PACKETCAPTURETHREAD_H
