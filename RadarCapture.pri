# 1. 确保能找到本目录下的头文件 RadarCapture
INCLUDEPATH += $$PWD

# 2. 将后端逻辑文件归类
SOURCES += \
    $$PWD/PacketCaptureThread.cpp \
    $$PWD/RadarCaptureManager.cpp \
    $$PWD/RadarOnlineDialog.cpp \
    $$PWD/RadarRealtimeParser.cpp \
	$$PWD/pcapextractsave.cpp \
    $$PWD/PacketProcessorThread.cpp

HEADERS += \
    $$PWD/LatencyMonitor.h \
    $$PWD/LatencyMonitorWidget.h \
    $$PWD/PacketCaptureThread.h \
    $$PWD/PacketProcessorThread.h \
    $$PWD/RadarCaptureManager.h \
    $$PWD/RadarOnlineDialog.h \
    $$PWD/RadarRealtimeParser.h \
	$$PWD/pcapextractsave.h \
    $$PWD/PacketQueue.h

# 3. 搬迁 Npcap 依赖（这样主 pro 就干净了）
INCLUDEPATH += $$PWD/npcap-sdk-1.16/Include
LIBS += -L$$PWD/npcap-sdk-1.16/Lib/x64
LIBS += -lws2_32 -lwpcap -lPacket
