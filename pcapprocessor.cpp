#include "pcapprocessor.h"
#include <QFile>
#include <QtEndian>
#include <QDebug>

#include <QDir>

#include <QRegularExpression>

#include <algorithm>
#include <QThread>

// 必须实现构造函数，即使它是空的
PcapProcessor::PcapProcessor(QObject *parent) : QObject(parent) {
}

// 必须实现析构函数
PcapProcessor::~PcapProcessor() {
}
bool PcapProcessor::run(const QString &fileName) {
    m_stop = false;    // 开始前重置
    m_working = true;  // 标记正在工作

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return false;

    uchar* begin = file.map(0, file.size());
    if (!begin) return false;

    uchar* curr = begin;
    uchar* end = begin + file.size();

    if (file.size() < sizeof(PcapGlobalHeader)) return false;
    curr += sizeof(PcapGlobalHeader);

    RadarReassembler reassembler;

    // 信号转发
    connect(&reassembler, &RadarReassembler::frameReady, this, &PcapProcessor::radarFrameParsed);

    // connect(&reassembler, &RadarReassembler::frameReady, [](quint32 id, const QByteArray& data){
    //     qDebug() << "------------------------------------";
    //     qDebug() << "Reassembled Frame ID:" << id << " Total Size:" << data.size();
    // });

    while (curr + sizeof(PcapPacketHeader) < end) {
        if (m_stop) break;

        PcapPacketHeader* pcapHdr = reinterpret_cast<PcapPacketHeader*>(curr);
        curr += sizeof(PcapPacketHeader);

        char* raw = reinterpret_cast<char*>(curr);
        int capLen = pcapHdr->incl_len;

        // 读取以太网类型字段
        uint16_t etherType = (raw[12] << 8) | raw[13];
        int ethHeaderLen = 14;

        // 如果是 0x8100，说明有 VLAN Tag，头部多 4 字节
        if (etherType == 0x8100) {
            ethHeaderLen = 18;
        }


        const int UDP_NETWORK_OFF = ethHeaderLen + 20 + 8;; // 以太网(14)+IP(20)+UDP(8)
        const int PACKET_HDR_SZ = 40;   // 每一包都有的 40 字节 Packet Header

        // 校验包长度是否合法
        if (capLen >= UDP_NETWORK_OFF + PACKET_HDR_SZ) {

            unsigned char* udpBase = reinterpret_cast<unsigned char*>(raw + ethHeaderLen + 20);
            uint16_t udpTotalLen = (udpBase[4] << 8) | udpBase[5]; // 大端转小端

            // 定位到 Packet Header
            RadarPacketHeader* rHdr = reinterpret_cast<RadarPacketHeader*>(raw + UDP_NETWORK_OFF);

            // 【关键修改点】：不再根据 Magic 额外跳转 72 字节
            // 无论是不是起始包，都只剥离 40 字节的 Packet Header
            // 这样起始包里紧跟在 40 字节后的 72 字节 Frame Header 就会被当作 payload 保留下来
            int businessDataOffset = UDP_NETWORK_OFF + PACKET_HDR_SZ;

            const char* payload = raw + businessDataOffset;
            int actualPayloadLen = udpTotalLen - 8 - PACKET_HDR_SZ;

            if (actualPayloadLen > 0) {
                reassembler.processPacket(rHdr, payload, actualPayloadLen);
            }
        }
        if (curr + capLen > end)
            break;

        curr += capLen;
    }

    file.unmap(begin);
    file.close();
    m_working = false; // 标记结束
    return !m_stop;    // 如果是因为停止而退出的，返回 false
}

void PcapProcessor::processBinFolder(const QString &folderPath) {

    QDir dir(folderPath);
    QStringList filters;
    filters << "*.bin";
    dir.setNameFilters(filters);

    // 1. 获取所有 .bin 文件（先不使用 QDir 的默认排序）
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files, QDir::NoSort);

    // 2. 正则表达式：精准捕获尾部下划线后的数字，如 "_13.bin" 中的 "13"
    QRegularExpression regex(R"(_(\d+)\.bin$)");

    // 3. 🌟 核心：使用 std::sort 强制按照数字大小进行升序排列 (1 -> 2 -> 13)
    std::sort(fileList.begin(), fileList.end(), [&regex](const QFileInfo &a, const QFileInfo &b) {
        auto matchA = regex.match(a.fileName());
        auto matchB = regex.match(b.fileName());

        if (matchA.hasMatch() && matchB.hasMatch()) {
            // 转成整数进行比较 (2 < 13 成立)
            return matchA.captured(1).toUInt() < matchB.captured(1).toUInt();
        }
        // 降级保护：如果命名不合规，用普通字符串排序
        return a.fileName() < b.fileName();
    });

    qDebug() << "PcapProcessor: Found and sorted" << fileList.size() << "bin files.";

    // 4. 严格按照排序后的顺序读取文件并同步发射信号
    for (const QFileInfo &fileInfo : fileList) {
        QString fileName = fileInfo.fileName();
        auto match = regex.match(fileName);

        if (!match.hasMatch()) {
            qWarning() << "ingore bin file name error:" << fileName;
            continue;
        }

        // 提取出纯数字作为硬件真实的 frameId
        quint32 frameId = match.captured(1).toUInt();

        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray rawBinData = file.readAll();
            file.close();

            // 🌟 核心复用点：直接触发你原本就有的信号！
            // 这样你原本内部的线程通道或者槽函数可以直接无感接收
            emit binFrameParsed(frameId, rawBinData);

            // 毫秒级微休眠，防止短时间内大量数据塞爆信号队列导致 UI 绘制卡顿
            QThread::msleep(1);
        }
    }

    qDebug() << "PcapProcessor: Finished injecting all BIN frames.";

    emit binFolderLoadingFinished();
}
