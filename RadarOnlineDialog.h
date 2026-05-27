#ifndef RADARONLINEDIALOG_H
#define RADARONLINEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "LatencyMonitorWidget.h"
#include <pcap.h>
#include <QStringList>
#include <winsock2.h> // Windows 下 inet_ntop 需要
#include <ws2tcpip.h>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCheckBox>
#include <QTextEdit>

// 获取网卡列表的静态函数
static QStringList getNetworkDevices() {
    QStringList devices;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs;

    if (pcap_findalldevs(&alldevs, errbuf) == -1) return devices;

    for (pcap_if_t *d = alldevs; d != nullptr; d = d->next) {
        QString ipAddr = "No IP";
        for (pcap_addr_t *a = d->addresses; a != nullptr; a = a->next) {
            if (a->addr && a->addr->sa_family == AF_INET) {
                char ipstr[INET_ADDRSTRLEN];
                struct sockaddr_in *sin = (struct sockaddr_in *)a->addr;
                if (inet_ntop(AF_INET, &sin->sin_addr, ipstr, sizeof(ipstr))) {
                    ipAddr = QString(ipstr);
                    break;
                }
            }
        }
        QString type = (d->flags & PCAP_IF_LOOPBACK) ? "[Loopback]" : "[Physical]";
        QString description = d->description ? QString(d->description) : "Unknown Device";
        QString displayName = QString("[%1] %2 - %3").arg(ipAddr, type, description);
        devices << QString("%1|%2").arg(d->name).arg(displayName);
    }

    pcap_freealldevs(alldevs);
    return devices;
}

class RadarOnlineDialog : public QDialog {
    Q_OBJECT
public:
    explicit RadarOnlineDialog(QWidget *parent = nullptr);
    ~RadarOnlineDialog();

    bool isMirrorChecked() const { return chkMirror->isChecked(); }

signals:
    void startCapture(QString device, QString ip, int port, QString cameraName, QSize res, double fps, int rotation);
    void stopCapture();
    void requestRecording(bool enabled);

    void requestMirror(bool mirrored);

private slots:
    void onStartClicked();
    void refreshDevices(); // 🚀 新增刷新槽函数
    void onCameraChanged(int index);
    void onResolutionChanged(int index);

private:
    void closeEvent(QCloseEvent *event) override;
    void populateDeviceCombo(); // 🚀 封装填充逻辑

    void updateUiState(bool isCapturing);

    QCheckBox *chkSave;
    QCheckBox *chkMirror;

    QComboBox *deviceCombo;
    QComboBox *cameraCombo;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QPushButton *btnRefresh; // 🚀 新增按钮指针
    QComboBox *resCombo;    // 分辨率
    QComboBox *fpsCombo;    // 帧率
    QComboBox *rotateCombo; // 旋转
    QTextEdit *portID; // portID
    QList<QCameraViewfinderSettings> currentSettingsList;


    LatencyMonitorWidget *monitorWidget;
};

#endif
