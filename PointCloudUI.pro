QT       += core gui multimedia multimediawidgets printsupport datavisualization concurrent network charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 windows debug_and_release
CONFIG -= console

# --- 解决 PCL 点云属性限制的关键配置 ---
DEFINES += BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
DEFINES += BOOST_MPL_LIMIT_VECTOR_SIZE=30

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# 检查是否使用了MSVC编译器
msvc {
    # /STACK:reserve[,commit]
    # Reserve 是保留的总栈大小，这里设置为 16MB (16 * 1024 * 1024)
    QMAKE_LFLAGS += /STACK:16777216
}

include(pcap_module/pcap_module.pri)
include(RadarCapture/RadarCapture.pri)
include(CameraImage/CameraImage.pri)
include(T16TR_BIN/T16TR_BIN.pri)

SOURCES += \
    FrameRangeDialog.cpp \
    MdfExtractDialog.cpp \
    QxtSpanSlider.cpp \
    RadarBinAnalysis.cpp \
    customradarPoint.cpp \
    loadedfilesdock.cpp \
    main.cpp \
    mainwindow.cpp \
    mdf2bin.cpp \
    pointcloud_loader.cpp \
    pointcloudcolorizer.cpp \
    radar_simulator.cpp \
    rspprocess.cpp \
    qcustomplot.cpp \
    # demoMex/dspProssing.c \
    # demoMex/accumulate.c  \
    # demoMex/CFAR.c  \
    # demoMex/common.c  \
    # demoMex/complexmath.c  \
    # demoMex/DOA.c  \
    # demoMex/fft.c  \
    # demoMex/mexdemo.c  \
    # demoMex/rvDecouple.c  \
    # demoMex/tx_order.c  \
    rspprocessor.cpp

HEADERS += \
    FrameRangeDialog.h \
    MdfExtractDialog.h \
    QxtSpanSlider.h \
    QxtSpanSlider_p.h \
    RadarBinAnalysis.h \
    customradarPoint.h \
    filterpointdata.h \
    loadedfilesdock.h \
    mainwindow.h \
    mdf2bin.h \
    pointcloud_loader.h \
    Radar_4T4RMain_Dll.h \
    pointcloudcolorizer.h \
    radar_simulator.h \
    rspprocess.h \
    rspprocessor.h \
    videoclass.h \
    #dspProssing.h \
    qcustomplot.h \
    # demoMex/dspProssing.h  \
    # demoMex/accumulate.h  \
    # demoMex/CFAR.h  \
    # demoMex/common.h  \
    # demoMex/complexmath.h  \
    # demoMex/DOA.h  \
    # demoMex/fft.h  \
    # demoMex/RadarSystemPara.h  \
    # demoMex/rvDecouple.h  \
    # demoMex/tx_order.h  \

FORMS += \
    MdfExtractDialog.ui \
    mainwindow.ui

INCLUDEPATH += $$PWD/rspheader

# message(PRO_FILE_PWD = $$PWD)

# 👉 先加载用户配置
CONFIG_PRI_DIR = $$PWD/configpri

exists($$CONFIG_PRI_DIR/user.pri): include($$CONFIG_PRI_DIR/user.pri)
message(PCL_ROOT = $$PCL_ROOT)

# 👉 再加载公共配置
include($$CONFIG_PRI_DIR/config.pri)


QMAKE_CXXFLAGS += /wd4100

DEFINES += VTK_USE_QVTKOPENGLWIDGET
DEFINES += QT_DEPRECATED_WARNINGS

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../qt-material-widgets/qt-material-widgets-master/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/components/release/ -lcomponents
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../qt-material-widgets/qt-material-widgets-master/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/components/debug/ -lcomponents

#INCLUDEPATH += $$PWD/../qt-material-widgets/qt-material-widgets-master/components
#DEPENDPATH += $$PWD/../qt-material-widgets/qt-material-widgets-master/components

RESOURCES += \
    myresource.qrc


QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /DEBUG

DISTFILES += \
    configpri/config.pri \
    configpri/user.pri
