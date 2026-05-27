#include "PacketCaptureThread.h"
#include <QDebug>

PacketCaptureThread::PacketCaptureThread(QString devName,
                                         QString dstIp,
                                         quint16 dstPort,
                                         // int frameLen,
                                         PacketQueue* queue,
                                         QObject* parent)
    : QThread(parent),
    deviceName(devName),
    filterIp(dstIp),
    filterPort(dstPort),
    // filterFrameLen(frameLen),
    packetQueue(queue),
    running(false)
{
}

void PacketCaptureThread::run()
{
    char errbuf[PCAP_ERRBUF_SIZE];

    // 1. 创建 pcap 句柄
    handle = pcap_create(deviceName.toStdString().c_str(), errbuf);
    if (!handle) {
        qDebug() << "pcap_create failed:" << errbuf;
        return;
    }

    // 2. 配置高级参数
    pcap_set_snaplen(handle, 65536);
    pcap_set_promisc(handle, 1);       // 混杂模式
    pcap_set_timeout(handle, 10);      // 设置较小的超时，方便快速响应退出请求
    pcap_set_immediate_mode(handle, 1); // 立即模式，不缓存，减小延迟
    pcap_set_buffer_size(handle, 16 * 1024 * 1024); // 16MB 内核缓冲区，防止丢包

    // 3. 激活设备
    if (pcap_activate(handle) != 0) {
        qDebug() << "pcap_activate failed";
        pcap_close(handle);
        handle = nullptr;
        return;
    }

    // 4. 构造并设置 BPF 过滤表达式
    // QString filterExp = QString("udp and dst host %1 and dst port %2")
    //                         .arg(filterIp)
    //                         .arg(filterPort);

    // QString filterExp;
    // if (filterIp.isEmpty()) {
    //     filterExp = QString("udp and dst port %1").arg(filterPort);
    // } else {
    //     filterExp = QString("udp and dst host %1 and dst port %2")
    //     .arg(filterIp)
    //         .arg(filterPort);
    // }

    QString filterExp;
    if (filterIp.isEmpty()) {
        // 同时兼容普通 UDP 和带 VLAN 的 UDP
        // 注意：vlan 关键字必须放在前面或使用 or 连接
        filterExp = QString("udp and dst port %1 or (vlan and udp and dst port %1)").arg(filterPort);
    } else {
        // 带有 IP 过滤的情况
        filterExp = QString("(udp and dst host %1 and dst port %2) or (vlan and udp and dst host %1 and dst port %2)")
                        .arg(filterIp)
                        .arg(filterPort);
    }

    struct bpf_program fcode;
    if (pcap_compile(handle, &fcode, filterExp.toStdString().c_str(), 1, PCAP_NETMASK_UNKNOWN) >= 0) {
        pcap_setfilter(handle, &fcode);
        pcap_freecode(&fcode);
    } else {
        qDebug() << "Filter compile failed:" << pcap_geterr(handle);
    }

    running = true;
    qDebug() << "Capture thread started for device:" << deviceName;

    // 5. 抓包循环
    while (!isInterruptionRequested())
    {
        // 每次处理 200 个包，超时或出错会返回
        int ret = pcap_dispatch(
            handle,
            200,
            packetHandler,
            reinterpret_cast<u_char*>(this));

        if (ret == -1) { // PCAP_ERROR
            qDebug() << "pcap_dispatch error:" << pcap_geterr(handle);
            break;
        }
        if (ret == -2) { // PCAP_ERROR_BREAK (调用了 pcap_breakloop)
            break;
        }
    }

    // 6. 统计并收尾
    running = false;

    if (m_pcapDumper) {
        pcap_dump_close(m_pcapDumper);
        m_pcapDumper = nullptr;
        qDebug() << "PCAP dumper closed in run() cleanup.";
    }


    struct pcap_stat stat;
    if (pcap_stats(handle, &stat) == 0) {
        kernelDrop = stat.ps_drop;
        qDebug() << "Capture finished. Total captured by kernel:" << stat.ps_recv
                 << " Dropped by kernel:" << stat.ps_drop;
    }

    if (handle) {
        pcap_close(handle);
        handle = nullptr;
    }

    // 🌟 重要：通知队列停止工作，唤醒可能正在 pop() 阻塞等待的解析线程
    if (packetQueue) {
        packetQueue->stop();
    }

    qDebug() << "Capture thread exited safely.";
}

void PacketCaptureThread::packetHandler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data) {
    PacketCaptureThread* self = reinterpret_cast<PacketCaptureThread*>(param);
    if (!self->running) return;

    // --- 1. 动态落盘逻辑 (PCAP Dumper) ---
        // m_isRecording 是由 MainWindow 通过 requestRecording 信号控制的开关
        if (self->m_isRecording) {
        if (!self->m_pcapDumper) {
            // 如果文件尚未打开，则打开它
            // 注意：m_savePath 是在 MainWindow 中生成的包含 yyyyMMdd_hhmmss 的完整路径
            self->m_pcapDumper = pcap_dump_open(self->handle, self->m_savePath.toStdString().c_str());

            if (!self->m_pcapDumper) {
                qDebug() << "Error: Could not open PCAP dumper for writing!";
            }
        }

        if (self->m_pcapDumper) {
            // 核心：直接将原始二进制包写入磁盘，不经过任何解析，效率最高
            pcap_dump((u_char*)self->m_pcapDumper, header, pkt_data);
        }
    } else {
        // 如果开关关闭且句柄还开着，说明刚刚停止录制，执行关闭刷盘
        if (self->m_pcapDumper) {
            pcap_dump_close(self->m_pcapDumper);
            self->m_pcapDumper = nullptr;
            qDebug() << "PCAP recording saved and closed.";
        }
    }

    // 一次性完成申请、拷贝、入队
    if (!self->packetQueue->enqueuePacket(pkt_data, header->caplen, header)) {
        // 队列满，丢包统计已在 enqueuePacket 内部完成
        return;
    }
    self->totalCaptured++;
}

// void PacketCaptureThread::startCapture()
// {
//     if (!isRunning())
//     {
//         start();
//     }
// }

void PacketCaptureThread::stopCapture()
{
    qDebug() << "Stopping capture...";
    requestInterruption();
    if (handle)
    {
        // qDebug() << "Stopping capture...";
        pcap_breakloop(handle);
    }
}

