#include "RadarCaptureManager.h"

RadarCaptureManager::RadarCaptureManager(QObject *parent)
    : QObject{parent}
{}

RadarCaptureManager::~RadarCaptureManager() {
    stop(); // 销毁时停止线程，防止崩溃
}
// ✅ 必须实现 stop 函数
void RadarCaptureManager::stop() {
    if (m_capThread) {
        m_capThread->requestInterruption();
        m_capThread->quit();
        m_capThread->wait();
        delete m_capThread;
        m_capThread = nullptr;
    }
    if (m_procThread) {
        m_procThread->requestInterruption();
        m_procThread->quit();
        m_procThread->wait();
        delete m_procThread;
        m_procThread = nullptr;
    }
    if (m_parser) {
        delete m_parser;
        m_parser = nullptr;
    }
    if (m_queue) {
        delete m_queue;
        m_queue = nullptr;
    }
}

void RadarCaptureManager::start(const QString &device, const QString &ip, int port) {
    stop();
    LatencyMonitor::instance().reset();

    if (m_procThread) {
        // 如果 procThread 还没销毁，重置内部的 radarWorker
        m_procThread->getWorker()->clear();
    }

    m_queue = new PacketQueue;
    m_capThread = new PacketCaptureThread(device, ip, port, m_queue);
    m_procThread = new PacketProcessorThread(m_queue);
    m_parser = new RadarRealtimeParser;


    // ✅ 关键：手动建立绑定关系
    m_procThread->setParser(m_parser);

    // ✅ 只需要这个连接（解析器 -> UI）
    connect(m_parser, &RadarRealtimeParser::cloudReady, this, &RadarCaptureManager::cloudReady);

    m_capThread->start();
    m_procThread->start();
}
