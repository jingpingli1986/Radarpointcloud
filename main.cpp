#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <exception>
#include <csignal>
#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include "QxtSpanSlider.h"
#include <QUuid>


QString g_instanceId;


void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QFile logFile("log.txt");
    static bool initialized = false;

    if (!initialized) {
        logFile.open(QFile::Append | QFile::Text);
        initialized = true;
    }

    QTextStream ts(&logFile);
    ts << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    switch (type) {
    case QtDebugMsg:    ts << "[DEBUG] "; break;
    case QtInfoMsg:     ts << "[INFO ] "; break;
    case QtWarningMsg:  ts << "[WARN ] "; break;
    case QtCriticalMsg: ts << "[ERROR] "; break;
    case QtFatalMsg:    ts << "[FATAL] "; break;
    }
    ts << msg << "\n";
    ts.flush();

    if (type == QtFatalMsg) {
        abort(); // fatal 时立即终止
    }
}


void signalHandler(int signal)
{
    QFile logFile("log.txt");
    if (logFile.open(QFile::Append | QFile::Text)) {
        QTextStream ts(&logFile);
        ts << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ")
           << "[CRASH] Signal " << signal << "\n";
    }
    ::exit(signal);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("YourCompany");
    QCoreApplication::setApplicationName("PointCloudUI");

    qRegisterMetaType<pcl::PointCloud<PointXYZRGBWithProperties>::Ptr>("pcl::PointCloud<PointXYZRGBWithProperties>::Ptr");
    qRegisterMetaType<uint32_t>("uint32_t");
    /*
    // 安装日志
    qInstallMessageHandler(messageHandler);

    // 捕捉崩溃信号
    signal(SIGSEGV, signalHandler); // 段错误
    signal(SIGABRT, signalHandler); // abort()
    signal(SIGFPE,  signalHandler); // 除零
    signal(SIGILL,  signalHandler); // 非法指令
    */



    QString appDir = QCoreApplication::applicationDirPath();
    QFile styleFile(appDir + "/style.qss");   // exe 同目录
    if(styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        a.setStyleSheet(style);
    }

    g_instanceId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    MainWindow w;

    w.setWindowTitle(QString("PointCloudUI [%1]").arg(g_instanceId.left(8)));
    w.show();




    try {
        return a.exec();
    } catch (std::exception &e) {
        qCritical() << "Unhandled exception:" << e.what();
    } catch (...) {
        qCritical() << "Unknown fatal error!";
    }

}
