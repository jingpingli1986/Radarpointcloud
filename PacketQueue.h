#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <QMutex>
#include <QWaitCondition>
#include <vector>
#include <atomic>
#include "pcap.h"
#include "LatencyMonitor.h"

constexpr int MAX_FRAME_LEN = 5000;
constexpr int QUEUE_CAPACITY = 10000; // 听取建议，缩小容量以减少 Cache Miss 和延迟

struct FramePacket {
    timeval ts;
    int frameLen;
    uint8_t frameData[MAX_FRAME_LEN];
};

class PacketQueue {
public:
    PacketQueue() : head(0), tail(0), count(0), stopped(false), queueFullDropCount(0) {
        buffer.resize(QUEUE_CAPACITY);
    }

    // 核心改进：原子化申请并占位
    FramePacket* enqueuePacket(const u_char* data, int len, const struct pcap_pkthdr* header) {
        QMutexLocker locker(&mutex);

        if (count >= QUEUE_CAPACITY) {
            queueFullDropCount++; // 统计丢包
            LatencyMonitor::instance().updateOverflowCount(queueFullDropCount.load());
            return nullptr;
        }

        FramePacket* pkt = &buffer[tail];
        pkt->ts = header->ts;

        // ✅ 修复：确保记录的长度与实际拷贝的长度严格一致


        int copyLen = (len > MAX_FRAME_LEN) ? MAX_FRAME_LEN : len;
        memcpy(pkt->frameData, data, copyLen);
        pkt->frameLen = copyLen;

        tail = (tail + 1) % QUEUE_CAPACITY;
        count++;
        cond.wakeOne();
        return pkt;
    }

    bool pop(FramePacket*& pkt) {
        QMutexLocker locker(&mutex);
        while (count == 0 && !stopped) cond.wait(&mutex);
        if (count == 0 && stopped) return false;

        pkt = &buffer[head];
        head = (head + 1) % QUEUE_CAPACITY;
        count--;
        return true;
    }

    void stop() {
        QMutexLocker locker(&mutex);
        stopped = true;
        cond.wakeAll();
    }

    uint64_t getDropCount() const { return queueFullDropCount.load(); }

private:
    std::vector<FramePacket> buffer;
    int head, tail, count;
    bool stopped;
    QMutex mutex;
    QWaitCondition cond;
    std::atomic<uint64_t> queueFullDropCount;
};
#endif
