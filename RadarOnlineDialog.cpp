#include "RadarOnlineDialog.h"

RadarOnlineDialog::RadarOnlineDialog(QWidget *parent) : QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Radar Online Control Panel");
    resize(1000, 600);

    // --- 1. 实例化所有组件 (必须最先执行) ---
    deviceCombo = new QComboBox(this);
    cameraCombo = new QComboBox(this);
    resCombo = new QComboBox(this);    // 移动到这里
    fpsCombo = new QComboBox(this);    // 移动到这里
    rotateCombo = new QComboBox(this); // 移动到这里

    chkMirror = new QCheckBox("Mirror", this); // 实例化镜像复选框
    chkMirror->setChecked(false); // 默认不镜像，或者根据你之前的测试结果设为 true


    btnRefresh = new QPushButton("Refresh", this);
    btnStart = new QPushButton("Start Online", this);
    btnStop = new QPushButton("Stop", this);
    monitorWidget = new LatencyMonitorWidget(this);

    portID = new QTextEdit(this);
    portID->setFixedHeight(30);   // 控制高度像输入框
    portID->setFixedWidth(100);
    portID->setText("8888");

    // --- 2. 设置布局 ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 第一行：网络和基础摄像头
    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    ctrlLayout->addWidget(new QLabel("Network:"));
    ctrlLayout->addWidget(deviceCombo, 1);
    ctrlLayout->addWidget(new QLabel("Port:"));
    ctrlLayout->addWidget(portID);

    ctrlLayout->addWidget(new QLabel("Camera:"));
    ctrlLayout->addWidget(cameraCombo, 1);
    ctrlLayout->addWidget(btnRefresh);
    ctrlLayout->addWidget(btnStart);
    ctrlLayout->addWidget(btnStop);
    mainLayout->addLayout(ctrlLayout);

    chkSave = new QCheckBox("开启同步录制 (PCAP & JPG)", this);
    chkSave->setEnabled(false); // 初始状态不可用，只有 Start 后才能点
    ctrlLayout->addWidget(chkSave);


    // 第二行：摄像头详细设置
    QHBoxLayout *camSettingsLayout = new QHBoxLayout();
    rotateCombo->addItem("0°", 0);
    rotateCombo->addItem("90°", 90);
    rotateCombo->addItem("180°", 180);
    rotateCombo->addItem("270°", 270);
    camSettingsLayout->addWidget(new QLabel("Resolution:"));
    camSettingsLayout->addWidget(resCombo, 1);
    camSettingsLayout->addWidget(new QLabel("FPS:"));
    camSettingsLayout->addWidget(fpsCombo, 1);
    camSettingsLayout->addWidget(new QLabel("Rotation:"));
    camSettingsLayout->addWidget(rotateCombo, 1);

    camSettingsLayout->addSpacing(10); // 加点间距
    camSettingsLayout->addWidget(chkMirror);

    mainLayout->addLayout(camSettingsLayout);

    mainLayout->addWidget(monitorWidget, 1);

    // --- 3. 绑定信号 (在填充数据之前绑定) ---
    connect(btnRefresh, &QPushButton::clicked, this, &RadarOnlineDialog::refreshDevices);
    connect(btnStart, &QPushButton::clicked, this, &RadarOnlineDialog::onStartClicked);
    connect(btnStop, &QPushButton::clicked, this, [this](){
        updateUiState(false); // 恢复 UI
        emit stopCapture();
    });

    connect(chkMirror, &QCheckBox::toggled, this, [this](bool checked){
        emit requestMirror(checked);
    });


    // 信号联动
    connect(cameraCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RadarOnlineDialog::onCameraChanged);
    connect(resCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RadarOnlineDialog::onResolutionChanged);

    // 绑定勾选逻辑
    connect(chkSave, &QCheckBox::toggled, this, [this](bool checked){
        if (checked) {
            chkSave->setText("🔴 正在录制至硬盘...");
            chkSave->setStyleSheet("color: red; font-weight: bold;");
        } else {
            chkSave->setText("开启同步录制 (PCAP & JPG)");
            chkSave->setStyleSheet("color: black; font-weight: normal;");
        }
        emit requestRecording(checked); // 发送给 MainWindow
    });

    // --- 4. 初始化加载数据 (最后一步) ---
    refreshDevices();
    btnStop->setEnabled(false);
}

void RadarOnlineDialog::updateUiState(bool isCapturing) {
    btnStart->setEnabled(!isCapturing);  // 采集时 Start 变灰
    btnStop->setEnabled(isCapturing);   // 采集时 Stop 可点

    chkMirror->setEnabled(!isCapturing);

    chkSave->setEnabled(isCapturing); // 只有开启了链路，才能点录制
    if (!isCapturing) chkSave->setChecked(false); // 停止采集时强制取消勾选


    // 工业建议：采集时锁定配置参数，防止中途修改导致逻辑混乱
    deviceCombo->setEnabled(!isCapturing);
    cameraCombo->setEnabled(!isCapturing);
    resCombo->setEnabled(!isCapturing);
    fpsCombo->setEnabled(!isCapturing);
    rotateCombo->setEnabled(!isCapturing);
    btnRefresh->setEnabled(!isCapturing);

}


// 🚀 填充/刷新 ComboBox 的核心逻辑
void RadarOnlineDialog::populateDeviceCombo() {
    // --- 1. 填充网卡 (原有逻辑) ---
    deviceCombo->clear();
    QStringList devs = getNetworkDevices();
    for(const QString& s : devs) {
        QStringList parts = s.split('|');
        if (parts.size() >= 2) {
            deviceCombo->addItem(parts[1], parts[0]);
        }
    }

    // --- 2. 填充摄像头 (新增逻辑) 🚀 ---
    cameraCombo->clear();
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo &info : cameras) {
        // 显示设备描述（如 "Logitech C922"），存储设备唯一标识符
        cameraCombo->addItem(info.description(), info.deviceName());
    }
}

void RadarOnlineDialog::refreshDevices() {
    // 1. 记录刷新前选中的 ID
    QString currentNetId = deviceCombo->currentData().toString();
    QString currentCamId = cameraCombo->currentData().toString(); // 🚀 记录当前摄像头

    // 2. 重新填充所有列表
    populateDeviceCombo();

    // 3. 恢复网卡选中状态
    int netIndex = deviceCombo->findData(currentNetId);
    if (netIndex != -1) {
        deviceCombo->setCurrentIndex(netIndex);
    }

    // 4. 恢复摄像头选中状态 🚀
    int camIndex = cameraCombo->findData(currentCamId);
    if (camIndex != -1) {
        cameraCombo->setCurrentIndex(camIndex);
    }
}

void RadarOnlineDialog::onStartClicked() {
    // 1. 立即锁定 UI
    updateUiState(true);


    LatencyMonitor::instance().reset();
    if (monitorWidget) {
        monitorWidget->clearChart();
    }

    QString selectedDevice = deviceCombo->currentData().toString();
    QString selectedCamera = cameraCombo->currentData().toString();
    QSize selectedRes = resCombo->currentData().toSize();
    double selectedFps = fpsCombo->currentData().toDouble();
    int selectedRotate = rotateCombo->currentData().toInt();
    if (selectedDevice.isEmpty()) {
        qDebug() << "No network device selected!";
        updateUiState(false);
        return;
    }

    int portnumber = portID->toPlainText().toInt();
    emit startCapture(selectedDevice, "", portnumber, selectedCamera, selectedRes, selectedFps, selectedRotate);
}

void RadarOnlineDialog::onCameraChanged(int index) {
    if (index < 0) return;

    resCombo->clear();
    fpsCombo->clear();
    currentSettingsList.clear(); // 成员变量，保存当前摄像头支持的组合

    QString cameraName = cameraCombo->currentData().toString();
    if (cameraName.isEmpty()) return;

    // 使用局部变量而不是堆分配，或者确保它安全销毁
    {
        QCamera tempCamera(cameraName.toUtf8());
        // 尝试等待一小段时间让硬件响应，或者使用静态获取方式
        tempCamera.load();
        currentSettingsList = tempCamera.supportedViewfinderSettings();
        tempCamera.unload(); // 显式卸载
    }

    QSet<QString> addedRes;
    for (const QCameraViewfinderSettings &setting : currentSettingsList) {
        QSize res = setting.resolution();
        QString resStr = QString("%1x%2").arg(res.width()).arg(res.height());

        if (!addedRes.contains(resStr)) {
            // 将 QSize 存入 Data，用于后续过滤
            resCombo->addItem(resStr, res);
            addedRes.insert(resStr);
        }
    }

    // 手动触发一次分辨率切换逻辑，填充初始帧率
    if (resCombo->count() > 0) {
        // 阻塞信号防止递归触发崩溃
        resCombo->blockSignals(true);
        onResolutionChanged(0);
        resCombo->blockSignals(false);
    }
}

void RadarOnlineDialog::onResolutionChanged(int index) {
    if (index < 0 || !resCombo) return; // 增加指针检查
    fpsCombo->clear();
    QVariant data = resCombo->currentData();
    if (!data.isValid()) return;

    QSize selectedRes = data.toSize();

    QSet<double> addedFps;
    for (const QCameraViewfinderSettings &setting : currentSettingsList) {
        // 只添加符合当前选中分辨率的帧率
        if (setting.resolution() == selectedRes) {
            double fps = setting.maximumFrameRate();
            if (!addedFps.contains(fps) && fps > 0) {
                fpsCombo->addItem(QString::number(fps), fps);
                addedFps.insert(fps);
            }
        }
    }
}


RadarOnlineDialog::~RadarOnlineDialog() {
    // 1. 停止所有下拉框触发的逻辑
    cameraCombo->blockSignals(true);
    resCombo->blockSignals(true);

    // 2. 显式清理
    currentSettingsList.clear();

    // 3. 如果 monitorWidget 有停止函数，调用它
    // monitorWidget->stop();

    qDebug() << "RadarOnlineDialog safely destroyed.";
}

void RadarOnlineDialog::closeEvent(QCloseEvent *event) {
    // 1. 如果正在录制，先模拟点击 Stop 按钮
    if (btnStop->isEnabled()) {
        qDebug() << "Closing: Stopping capture and saving...";

        // 强制取消勾选录制，触发 MainWindow 的 handleRecordingRequest(false)
        if (chkSave->isChecked()) {
            chkSave->setChecked(false);
        }

        // 触发停止采集信号
        emit stopCapture();
    }

    // 2. 给予后端一点点时间处理文件刷盘（可选）
    // 或者通过信号量等待停止完成

    event->accept(); // 允许关闭
}
