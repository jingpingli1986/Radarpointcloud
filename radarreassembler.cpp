
// RadarReassembler.cpp
#include "radarreassembler.h"
#include <QtEndian>
#include <QDebug>

RadarReassembler::RadarReassembler(QObject *parent) : QObject(parent) {
    m_buffer.resize(MAX_FRAME_SIZE);
    resetInternal();
}

void RadarReassembler::processPacket(const RadarPacketHeader* header, const char* payload, int len) {
    // 1. 解析头部（小端）
    quint32 magic   = qFromLittleEndian<quint32>(header->frameStartMagic);
    quint32 frameId = qFromLittleEndian<quint32>(header->frameID);

    // ==========================================
    // 维度 1: 起始包检测 (0x00BE0001 或 0xEFBE0001)
    // ==========================================
    if (magic == 0x00BE0001 || magic == 0xEFBE0001) {

        // 检查上一帧是否残缺 (对齐 Online: if (worker.isCollecting()))
        if (m_state == State::Collecting) {
            // 如果是同一帧的重复 Start 包，根据 Online 逻辑通常覆盖或忽略，此处选择重置重新开始
            if (m_currentFrameId != frameId) {
                qWarning() << "[WARN] Incomplete frame dropped. ID:" << m_currentFrameId;
            }
        }

        // 重置 Buffer 指针并开启收集
        m_writeOffset = 0;
        m_currentFrameId = frameId;
        m_state = State::Collecting;
    }

    // ==========================================
    // 维度 2: 数据写入 (只有 Collecting 状态才处理)
    // ==========================================
    if (m_state == State::Collecting) {
        // 校验当前包是否属于正在收集的 FrameID
        // 离线解析若不校验这个，可能会把 Pcap 中乱序或交织的其他帧数据塞进来
        if (frameId == m_currentFrameId) {
            if (m_writeOffset + len <= MAX_FRAME_SIZE) {
                memcpy(m_buffer.data() + m_writeOffset, payload, len);
                m_writeOffset += len;
            } else {
                qWarning() << "[ERROR] Buffer overflow for Frame:" << frameId;
                m_state = State::Idle;
                return;
            }
        }
    } else {
        // 还没收到起始包，所有的内容包 (0x00000001) 都会走到这里被丢弃
        // 对齐 Online: if (worker.isCollecting()) { ... } else { return; }
        return;
    }

    // ==========================================
    // 维度 3: 结束包检测 (0xEF000001 或 0xEFBE0001)
    // ==========================================
    if (magic == 0xEF000001 || magic == 0xEFBE0001) {
        // 只有当前正在收集且 ID 匹配时才结算
        if (m_state == State::Collecting && frameId == m_currentFrameId) {

            // 校验长度是否合理 (对齐 Online: > 3000000)
            if (m_writeOffset > 3000000) {
                // 发射信号，注意 QByteArray 拷贝构造会共享内存，开销较小
                emit frameReady(m_currentFrameId, QByteArray(m_buffer.constData(), m_writeOffset));
            } else {
                qWarning() << "[DROP] Frame too small:" << m_writeOffset << "ID:" << frameId;
            }

            // 无论成功与否，结算后重置状态机
            resetInternal();
        }
    }
}

void RadarReassembler::flush() {
    if (m_state == State::Collecting && m_writeOffset > 3000000) {
        emit frameReady(m_currentFrameId, QByteArray(m_buffer.constData(), m_writeOffset));
    }
    resetInternal();
}

void RadarReassembler::reset() {
    resetInternal();
}

void RadarReassembler::resetInternal() {
    m_writeOffset = 0;
    m_currentFrameId = 0xFFFFFFFF;
    m_state = State::Idle;
}

void RadarReassembler::processFragment(const char* payload, int len) {
    // 分片包处理完全对齐 Online 逻辑：
    // 如果正在收集中，说明这是上一包的 IP 分片，直接追加到末尾
    if (m_state == State::Collecting) {
        if (m_writeOffset + len <= MAX_FRAME_SIZE) {
            memcpy(m_buffer.data() + m_writeOffset, payload, len);
            m_writeOffset += len;
        }
    }
}
