// RadarReassembler.h
#ifndef RADAR_REASSEMBLER_H
#define RADAR_REASSEMBLER_H

#include <QObject>
#include <QByteArray>
#include "RadarProtocol.h"




class RadarReassembler : public QObject {
    Q_OBJECT
public:
    explicit RadarReassembler(QObject *parent = nullptr);
    void processPacket(const RadarPacketHeader* header, const char* payload, int len);
    void flush();
    void reset();
signals:
    void frameReady(quint32 frameId, const QByteArray &data);

private:
    enum class State {
        Idle,
        Collecting,
        Error
    };
    void resetInternal();

    QByteArray m_buffer;
    int m_writeOffset = 0;
    quint32 m_currentFrameId = 0xFFFFFFFF;
    quint16 m_lastPktId = 0;
    static constexpr int MAX_FRAME_SIZE = 4 * 1024 * 1024; // 4MB
    State m_state = State::Idle;
    void processFragment(const char* payload, int len);
};

#endif
