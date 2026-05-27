#include "mainwindow.h"
#include "MdfExtractDialog.h"
#include "ui_mainwindow.h"
#include <QProxyStyle>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <QtConcurrent>
#include "videoclass.h"
#include <QMediaPlayer>   // 👈 必须显式加上这一行！
#include <QVideoWidget>   // 如果用到了相关组件也加上


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    //, cloud(new pcl::PointCloud<pcl::PointXYZRGB>)
{



    ui->setupUi(this);
    // std::string mdf_file = "C:/Users/JPN1WX/Desktop/QT/PointCloudUI/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/8x8data/Recorder_2025-11-20_16-11-06_radar_test_data.mf4";
    //     std::string output_dir = "C:/Users/JPN1WX/Desktop/QT/PointCloudUI/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/8x8data/"; // 建议使用子目录

    //     size_t start_f = 0;
    //     size_t end_f = 10; // 提取帧 850 到 859

    //     // 调用修改后的函数
    //     size_t saved_count = readallmdftobin_by_frame(mdf_file, output_dir, start_f, end_f, "VideoRawdata_VC0"); // 确认视频通道名称


    // --- 初始边界设置为 -150 到 150 ---
        currentCloudBounds[0] = -150.0; // minX
        currentCloudBounds[1] = 150.0;  // maxX
        currentCloudBounds[2] = -150.0; // minY
        currentCloudBounds[3] = 150.0;  // maxY
        currentCloudBounds[4] = 0.0;    // minZ
        currentCloudBounds[5] = 0.0;    // maxZ

        // 中心点初始化
        cloudCenterX = 0.0;
        cloudCenterY = 0.0;

        // 初始缩放比例：建议设为比 150 略大，以便看到全貌
        suggestedParallelScale = 180.0;


    // 1. 设置父对象，使其进入渲染窗口内部
    ui->XYZ45->setParent(ui->qvtkWidget);
    ui->XYAziInitial->setParent(ui->qvtkWidget);

    // 2. 初始位置定位 (例如：左上角)
    ui->XYAziInitial->move(10, 5); // 放在 XYZ45 旁边
    ui->XYZ45->move(10, 50);


    // 3. 提升层级，防止被 OpenGL 背景遮挡
    ui->XYZ45->raise();
    ui->XYAziInitial->raise();

    // 4. 显示按钮 (改变父级后需要显式 show)
    ui->XYZ45->show();
    ui->XYAziInitial->show();
    // 一些暂时不用的按钮隐藏
    ui->saveMultiFrameButton->hide();

    ui->prevFrameButton->setIcon(
        style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->FramePlayButton->setIcon(
        style()->standardIcon(QStyle::SP_MediaPlay));
    ui->nextFrameButton->setIcon(
        style()->standardIcon(QStyle::SP_MediaSkipForward));

    ui->prevFrameButton->setIconSize(QSize(24, 24));
    ui->FramePlayButton->setIconSize(QSize(32, 32));
    ui->nextFrameButton->setIconSize(QSize(24, 24));

    ui->prevFrameButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    ui->FramePlayButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    ui->nextFrameButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    ui->prevFrameButton->setAutoRaise(false);
    ui->FramePlayButton->setAutoRaise(false);
    ui->nextFrameButton->setAutoRaise(false);

    connect(ui->FramePlayButton, &QPushButton::clicked,
            this, &MainWindow::on_playPauseButton_clicked);

    setCentralWidget(nullptr);

    this->setDockNestingEnabled(true);

    loadedFilesDock = new LoadedFilesDock(this);

    // addDockWidget(Qt::RightDockWidgetArea, loadedFilesDock);


    // 允许 Dock 之间共享边缘
    this->setAnimated(true);
    // addDockWidget(Qt::LeftDockWidgetArea, ui->PointCloudDock);
    // addDockWidget(Qt::LeftDockWidgetArea, ui->ControlDock);
    // splitDockWidget(ui->ControlDock, ui->PointCloudDock, Qt::Horizontal);
    // addDockWidget(Qt::RightDockWidgetArea, ui->Video);
    // addDockWidget(Qt::RightDockWidgetArea, ui->RD);
    // addDockWidget(Qt::RightDockWidgetArea, ui->PointFilters);
    // addDockWidget(Qt::BottomDockWidgetArea, ui->PointData);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // 设置允许停靠的区域（防止它不小心被拽到左右两边回不来）

    // 将 RD 放在 Video 的下方
    // splitDockWidget(ui->Video, ui->RD, Qt::Vertical);
    // 将 PointFilters 放在 RD 的下方
    // splitDockWidget(ui->RD, ui->PointFilters, Qt::Vertical);

    // resizeDocks({ui->ControlDock, ui->PointCloudDock, ui->Video}, {50, 600, 250}, Qt::Horizontal);
    ui->ControlDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    // 设置初始显示状态：PointCloud 显示，Video 不显示
    // ui->RDimage->setChecked(true);
    // ui->DisplayVideo->setChecked(true);
    // ui->selectpiontdata->setChecked(true);
     pcMinMax = nullptr;

    //暂时隐藏一下按钮
    //ui->readRadar4T4Bin->hide();
    //ui->loadButton->hide();
    //ui->saveButton->hide();


    // 让窗口最大化打开
    // this->showMaximized();
   // setupVTKWidget();
   // updateLayout();

    //setupBackgroud();
    // 连接勾选框信号到槽函数
    // RD dock 与 checkbox 同步
    connect(ui->RDimage, &QCheckBox::toggled, this, [this](bool checked){
        ui->RD->setVisible(checked);
    });

    connect(ui->RD, &QDockWidget::visibilityChanged, this, [this](bool visible){
        ui->RDimage->blockSignals(true);
        ui->RDimage->setChecked(visible);
        ui->RDimage->blockSignals(false);
    });

    // Video dock 与 checkbox 同步
    connect(ui->DisplayVideo, &QCheckBox::toggled, this, [this](bool checked){
        ui->Video->setVisible(checked);
    });

    connect(ui->Video, &QDockWidget::visibilityChanged, this, [this](bool visible){
        ui->DisplayVideo->blockSignals(true);
        ui->DisplayVideo->setChecked(visible);
        ui->DisplayVideo->blockSignals(false);
    });

    // PointData dock 与 checkbox 同步
    connect(ui->selectpiontdata, &QCheckBox::toggled, this, [this](bool checked){
        ui->PointData->setVisible(checked);
    });

    connect(ui->PointData, &QDockWidget::visibilityChanged, this, [this](bool visible){
        ui->selectpiontdata->blockSignals(true);
        ui->selectpiontdata->setChecked(visible);
        ui->selectpiontdata->blockSignals(false);
    });

    // PointFilter dock 与 checkbox 同步
    connect(ui->pointfilter, &QCheckBox::toggled, this, [this](bool checked){
        ui->PointFilters->setVisible(checked);
    });

    connect(ui->PointFilters, &QDockWidget::visibilityChanged, this, [this](bool visible){
        ui->pointfilter->blockSignals(true);
        ui->pointfilter->setChecked(visible);
        ui->pointfilter->blockSignals(false);
    });

    // if (!ui->pointfilter->isChecked()) {
    //     ui->PointFilters->hide();
    //     // 或者使用 setVisible(ui->checkBoxName->isChecked());
    // }

    // PointCloud dock 与 checkbox 同步
    connect(ui->Pointcloutviewer, &QCheckBox::toggled, this, [this](bool checked){
        ui->PointCloudDock->setVisible(checked);
    });

    connect(ui->PointCloudDock, &QDockWidget::visibilityChanged, this, [this](bool visible){
        ui->Pointcloutviewer->blockSignals(true);
        ui->Pointcloutviewer->setChecked(visible);
        ui->Pointcloutviewer->blockSignals(false);
    });

    // 初始化：确保启动时状态一致
    ui->PointCloudDock->setVisible(ui->Pointcloutviewer->isChecked());


    connect(ui->checkBoxSelectMode, &QCheckBox::toggled, this, [=](bool checked) {
        selectionModeEnabled = checked;
    });

    connect(ui->comboBoxSelectedPoints, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_selectedPointChanged);


 //    connect(ui->frameSlider, &QSlider::valueChanged, this, [this](int value){

 //        // if (isUpdatingFromVideo) return; // 避免循环

 //            isUpdatingFromCloud = true;

 // if (isRadar8x8Mode) {
 //        // 8×8 模式
 //        if (value < 0 || value >= frames8x8.size()) return;
 //        currentFrameIndex = value;
 //        auto &frame = frames8x8[currentFrameIndex];

 //        satecloud = frame.Radar8T8Point.cloud;
 //        displayPointCloud();

 //        // 更新 RD Map
 //        rdMapPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
 //        showRDMap(rdMapPlot, frame.Radar8T8Point.rdmap.data());

 //        // 更新时间标签
 //        ui->pointruntime->setText(QString::number(frame.indexno ));
 //        ui->pointstarttime->setText(QString::fromStdString(
 //            convertTimestamp(frame.start_time_ns + frame.timestamp_ns)));

 //        // 视频同步
 //               if (m_player) {
 //                   // 计算当前帧相对于第一帧的绝对偏移时间（毫秒）
 //                   long long t0 = frames8x8[0].timestamp_ns;
 //                   long long t_curr = frame.timestamp_ns;
 //                   qint64 cloudTimeMs = (t_curr - t0) / 1000000; // 纳秒转毫秒

 //                   isUpdatingFromCloud = true;      // ⚡ 避免 slider 循环
 //                    m_player->setPosition(cloudTimeMs);
 //                    isUpdatingFromCloud = false;
 //               }

 //               if (!CarData.empty() && !CarSRSYawData.empty()) {
 //                   // qDebug()<<"ccc";
 //                   updateCarDataDisplay(frames8x8[currentFrameIndex].timestamp_ns);
 //               }
 //    } else {
 //        // if(currentFrameIndex >= 0 && currentFrameIndex < frames.size() &&
 //        //     currentFrameIndex < multiFrames.size()) {
 //        //     // 安全访问数组
 //        // } else {
 //        //     qDebug() << "Frame index out of range:" << currentFrameIndex;
 //        //     return;
 //        // }

 //        if (value < 0 || value >= multiFrames.size()) return; // 简化边界检查
 //        currentFrameIndex = value;
 //        satecloud = multiFrames[currentFrameIndex];
 //        // if (!isProgrammaticallyChangingSlider) {
 //        //     frameBuffer.clear();
 //        //     lastAccumulatedCloud = nullptr;
 //        // }


 //        displayPointCloud();

 //        // 更新时间标签
 //        ui->pointruntime->setText(QString::number(frames[currentFrameIndex].indexno ));
 //        ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frames[currentFrameIndex].start_time_ns+frames[currentFrameIndex].timestamp_ns)));

 //        if (m_player) {
 //            qint64 cloudTimeMs = static_cast<qint64>(frames[currentFrameIndex].timestamp_ns / 1000000);

 //            isUpdatingFromCloud = true;
 //                        m_player->setPosition(cloudTimeMs);
 //                        isUpdatingFromCloud = false;
 //        }
 //        if(!CarData.empty() && !CarSRSYawData.empty()){
 //        updateCarDataDisplay(frames[currentFrameIndex].start_time_ns+frames[currentFrameIndex].timestamp_ns);
 //        }
 //    }

 //    isUpdatingFromCloud = false;
 //    });

    connect(ui->frameSlider, &QSlider::valueChanged, this, [this](int value){
        // 1. 严格检查拦截位
        if (isUpdatingFromVideo || isUpdatingFromCloud) return;

        // 2. 立即上锁
        isUpdatingFromCloud = true;

        if (isRadar8x8Mode) {
            if (value < 0 || value >= (int)frames8x8.size()) {
                isUpdatingFromCloud = false;
                return;
            }
            currentFrameIndex = value;
            auto &frame = frames8x8[currentFrameIndex];


            bool isBinOfflineMode = (frame.start_time_ns == 0);

                    // ==========================================
                    // A 分支：当前帧【有】合法点云数据 -> 全量渲染
                    // ==========================================
                    if (frame.Radar8T8Point.cloud != nullptr && !frame.Radar8T8Point.cloud->empty()) {
                        // --- 渲染点云 ---
                        satecloud = frame.Radar8T8Point.cloud;
                        displayPointCloud();

                        // --- 渲染 RD Map ---
                        rdMapPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
                        showRDMap(rdMapPlot, frame.Radar8T8Point.rdmap.data());

                        // --- 车速/偏航率 UI 更新 ---
                        if (!CarData.empty() && !CarSRSYawData.empty()) {
                            updateCarDataDisplay(frame.timestamp_ns);
                        } else {
                            m_current8x8Speed = frame.egoSpeed;
                            m_current8x8YawRate = frame.egoYawRate;
                            updateCarDataDisplay(frame.timestamp_ns);
                        }
                        ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frame.start_time_ns + frame.timestamp_ns)));
                    }
                    // ==========================================
                    // B 分支：当前帧【无】雷达数据 (空洞过渡期) -> 优雅清理，绝不 return
                    // ==========================================
                    else {
                        if (pointCloudActor) {
                            mainRenderer->RemoveActor(pointCloudActor);
                            pointCloudActor = nullptr;
                            if (renderWindow) renderWindow->Render(); // 强刷 3D 舞台，清空残影
                        }
                        ui->pointstarttime->setText("当前视频段无雷达数据");
                    }

                    // ==========================================
                    // C 公共刷新：无论是空洞还是有效帧，视频和帧号文本必须跟着走！
                    // ==========================================
                    ui->pointruntime->setText(QString::number(frame.indexno));

                    // 🌟 查找临近相机图片（保留你原有的逻辑）
                    qint64 absoluteTimeNs = frame.start_time_ns + frame.timestamp_ns;
                    QString imgPath = findClosestImage(absoluteTimeNs);
                    if (!imgPath.isEmpty()) {
                        QPixmap pix(imgPath);
                        if (!pix.isNull()) {
                            ui->videoLabel->setPixmap(pix.scaled(ui->videoLabel->size(),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        }
                    }

                    // --- 🌟 视频同步跳转控制 ---
                    if (m_player) {
                        qint64 cloudTimeMs = 0;
                        if (isBinOfflineMode) {
                            // 1️⃣ 离线模式：视频是连续的，必须用“当前帧号 * 50ms”与定时器绝对死锁对齐
                            cloudTimeMs = static_cast<qint64>(currentFrameIndex) * 50;
                        } else {
                            // 2️⃣ 实时网络抓包模式：保持你原有的时间戳差值计算
                            long long t0 = (!frames8x8.empty()) ? frames8x8[0].timestamp_ns : 0;
                            cloudTimeMs = (frame.timestamp_ns - t0) / 1000000;
                        }
                        m_player->setPosition(cloudTimeMs);
                    }
        } else {
            // --- 非 8x8 模式 ---
            if (value < 0 || value >= (int)multiFrames.size()) {
                isUpdatingFromCloud = false;
                return;
            }
            currentFrameIndex = value;
            satecloud = multiFrames[currentFrameIndex];
            displayPointCloud();

            ui->pointruntime->setText(QString::number(frames[currentFrameIndex].indexno));
            ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frames[currentFrameIndex].start_time_ns + frames[currentFrameIndex].timestamp_ns)));

            if (m_player) {
                qint64 cloudTimeMs = frames[currentFrameIndex].timestamp_ns / 1000000;
                m_player->setPosition(cloudTimeMs);
            }

            if (!CarData.empty() && !CarSRSYawData.empty()) {
                updateCarDataDisplay(frames[currentFrameIndex].start_time_ns + frames[currentFrameIndex].timestamp_ns);
            }
        }

        // 3. ⚡ 最关键的改动：延迟释放锁
        // 确保在视频跳转稳定之前，不接受任何来自视频端的反向调用
        QTimer::singleShot(100, [this](){
            isUpdatingFromCloud = false;
        });
    });

    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->qvtkWidget->setRenderWindow(renderWindow);


    mainRenderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(mainRenderer);

    auto interactor = ui->qvtkWidget->renderWindow()->GetInteractor();
    interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());

    //interactor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyleUser>::New());
    // 播放video
    m_surface = new VideoSurface(ui->videoLabel, this);
    m_player = new QMediaPlayer(this);
    m_player->setVideoOutput(m_surface);

    // video slider 与播放器连接
    connect(m_player, &QMediaPlayer::durationChanged, this, [=](qint64 duration){
        ui->horizontalSlider->setMaximum(static_cast<int>(duration));
        int ms = duration / 1000;
        ui->labelDuration->setText(formatTime(duration));

    });

    connect(m_player, &QMediaPlayer::positionChanged, this, [=](qint64 position){
        ui->horizontalSlider->setValue(static_cast<int>(position));
        ui->labelPosition->setText(formatTime(position));
        if (!isUpdatingFromCloud && !multiFrames.empty()) { // ⚡ 避免循环
                isUpdatingFromVideo = true;
                updatePointCloudFromVideo(position);            // 点云跟随视频
                isUpdatingFromVideo = false;
            }








    });


    connect(ui->horizontalSlider, &QSlider::valueChanged, this, [this](int value){
        if (isUpdatingFromCloud) return;

        isUpdatingFromVideo = true;

        m_player->setPosition(value);            // 视频跳转
        updatePointCloudFromVideo(value);        // 点云同步

        qint64 relativeTimeNs = static_cast<qint64>(value) * 1000000;

            // --- A. 同步图片 (增加导入判断) ---
            // 只有当 Map 不为空且 radar 容器有效时才执行
            if (!m_imageTimeMap.isEmpty()) {
                qint64 absoluteTimeNs = frames8x8[0].start_time_ns + relativeTimeNs;
                QString imgPath = findClosestImage(absoluteTimeNs);

                if (!imgPath.isEmpty()) {
                    QPixmap pix(imgPath);
                    if (!pix.isNull()) {
                        ui->videoLabel->setPixmap(pix.scaled(ui->videoLabel->size(),
                            Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    }
                }
            } else {
                // 可选：如果没图，可以清空 Label 或显示“未加载视频”
                // ui->videoLabel->setText("No Video Loaded");
            }

            // --- B. 同步点云 (保持不变) ---
            int radarIndex = findClosestRadarFrame(relativeTimeNs);
            if (radarIndex != -1) {
                currentFrameIndex = radarIndex;
                satecloud = frames8x8[radarIndex].Radar8T8Point.cloud;
                displayPointCloud();

                ui->frameSlider->blockSignals(true);
                ui->frameSlider->setValue(radarIndex);
                ui->frameSlider->blockSignals(false);
            }



        isUpdatingFromVideo = false;



    });

    connect(ui->horizontalSlider, &QSlider::sliderMoved, this, [=](int value){
        m_player->setPosition(static_cast<qint64>(value));
    });


    connect(ui->comboBoxSelectedPoints, QOverload<int>::of(&QComboBox::activated),
            this, [=](int index){
                QVariant data = ui->comboBoxSelectedPoints->itemData(index, Qt::UserRole);
                if (data.isValid())
                    ui->labelPointInfo->setText(data.toString());
                else
                    ui->labelPointInfo->clear();
            });

    // 初始化 PCL 可视化器并绑定到 VTK 渲染窗口


    highlightedPoints = vtkSmartPointer<vtkPoints>::New();
    highlightedPolyData = vtkSmartPointer<vtkPolyData>::New();
    highlightedPolyData->SetPoints(highlightedPoints);

    vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    glyphFilter->SetInputData(highlightedPolyData);
    glyphFilter->Update();

    highlightedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    highlightedMapper->SetInputConnection(glyphFilter->GetOutputPort());

    highlightedActor = vtkSmartPointer<vtkActor>::New();
    highlightedActor->SetMapper(highlightedMapper);
    highlightedActor->GetProperty()->SetColor(1, 0, 0);      // 红色高亮
    highlightedActor->GetProperty()->SetPointSize(8);        // 点大小
    highlightedActor->SetPickable(false);
    mainRenderer->AddActor(highlightedActor);

    // 初始化点选器
    pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    pointPicker->SetTolerance(0.005);
    pointPicker->PickFromListOn();
    //interactor->SetPicker(pointPicker);


    cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    cellPicker->SetTolerance(0.05); // 容差，0.01~0.05可调
    //ui->qvtkWidget->renderWindow()->GetInteractor()->SetPicker(cellPicker);

    setupVTKWidget();

    //SatalitePointCloud();

    //satecloud = customdata.SateliteRadarData();



    playbackTimer = new QTimer(this);

    connect(playbackTimer, &QTimer::timeout, this, [this]() {
        if (frames8x8.empty() && multiFrames.empty()) return;

        if (isRadar8x8Mode) {
            // =========================================================================
            // 8×8 模式大分支
            // =========================================================================
            int maxFrame = ui->frameSlider->maximum();
            int minFrame = ui->frameSlider->minimum();
            bool isBinOfflineMode = (!frames8x8.empty() && frames8x8[0].start_time_ns == 0);

            if (isBinOfflineMode) {
                // =========================================================================
                // 【分流 B-1】: 离线 BIN 模式（AVI完整，BIN片段）—— 视频均匀时钟驱动
                // =========================================================================

                // 🌟 1. 开头哨兵拦截：如果由于外部意外操作导致超出边界，立刻归位死锁
                if (currentFrameIndex > maxFrame || currentFrameIndex < minFrame) {
                    playbackTimer->stop();
                    currentFrameIndex = minFrame;
                    isPlaying = false;
                    ui->playPauseButton->setText("Play");
                    ui->FramePlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

                    isUpdatingFromCloud = true; // 定时器控制，临时锁死
                    ui->frameSlider->setValue(currentFrameIndex);
                    ui->pointruntime->setText(QString("Frame: %1").arg(currentFrameIndex));
                    isUpdatingFromCloud = false; // 顺手解锁

                    if (m_player) m_player->stop();
                    return;
                }

                // 🌟 2. 正常渲染流程开始 -> 全局死锁保护，防止 valueChanged 重入
                isUpdatingFromCloud = true;

                ui->frameSlider->setValue(currentFrameIndex);
                ui->pointruntime->setText(QString("Frame: %1").arg(currentFrameIndex));

                // 下标映射检索
                if (currentFrameIndex < frames8x8.size() &&
                    frames8x8[currentFrameIndex].Radar8T8Point.cloud != nullptr &&
                    !frames8x8[currentFrameIndex].Radar8T8Point.cloud->empty() &&
                    frames8x8[currentFrameIndex].indexno == static_cast<long long>(currentFrameIndex)) {

                    auto &frame = frames8x8[currentFrameIndex];
                    satecloud = frame.Radar8T8Point.cloud;

                    displayPointCloud(); // 刷新 VTK 渲染

                    if (!frame.Radar8T8Point.rdmap.empty()) {
                        showRDMap(rdMapPlot, frame.Radar8T8Point.rdmap.data());
                    }

                    ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frame.timestamp_ns)));
                    if (!CarData.empty() && !CarSRSYawData.empty()) { // 保持你原有的车辆数据判空
                        updateCarDataDisplay(frame.timestamp_ns);
                    }
                }
                else {
                    // 当前视频帧是空白过渡期，移除历史点云残影
                    if (pointCloudActor) {
                        mainRenderer->RemoveActor(pointCloudActor);
                        pointCloudActor = nullptr;
                        if (renderWindow) renderWindow->Render();
                    }
                    ui->pointstarttime->setText("no data");
                }

                // 通过绝对帧号直接强推视频画面（50ms一帧）
                if (m_player) {
                    qint64 targetVideoPosMs = static_cast<qint64>(currentFrameIndex) * 50;
                    m_player->setPosition(targetVideoPosMs);
                }

                // 🌟 3. 精准终点判定：如果当前播完的已经是最后一帧，就地优雅停靠！
                if (currentFrameIndex >= maxFrame) {
                    playbackTimer->stop();
                    // 此时界面完美定格在 maxFrame，给后台 currentFrameIndex 备好料
                    currentFrameIndex = minFrame;
                    isPlaying = false;
                    ui->playPauseButton->setText("Play");
                    ui->FramePlayButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

                    isUpdatingFromCloud = false; // 🌟 必须释放安全锁，还给手动拖拽
                    return; // 🎯 切断后续心跳，完美结束
                }

                // 🌟 4. 未到终点，时钟驱动步进
                currentFrameIndex++;
                isUpdatingFromCloud = false; // 🌟 释放安全锁，准许外部事件响应
                playbackTimer->start(50);
            }
            else {
                // =========================================================================
                // 【分流 B-2】: 实时抓包 PCAP 模式（历史原有逻辑，稳如磐石）
                // =========================================================================
                if (currentFrameIndex >= frames8x8.size()-1) {
                    playbackTimer->stop();
                    currentFrameIndex = 0;
                    isPlaying = false;
                    ui->playPauseButton->setText("Play");
                    if (m_player) m_player->stop();
                    return;
                }

                auto &frame = frames8x8[currentFrameIndex];
                satecloud = frame.Radar8T8Point.cloud;
                displayPointCloud();
                showRDMap(rdMapPlot, frame.Radar8T8Point.rdmap.data());

                // 🌟 修复隐患：网络模式下的自增setValue也应该享受锁保护
                isUpdatingFromCloud = true;
                ui->frameSlider->setValue(currentFrameIndex);
                isUpdatingFromCloud = false;

                ui->pointstarttime->setText(QString::fromStdString(
                    convertTimestamp(frame.start_time_ns + frame.timestamp_ns)));
                ui->pointruntime->setText(QString::number(frame.indexno));

                qint64 cloudTimeMs = 0;

                if (currentFrameIndex == 0) {
                    cloudTimeMs = 0;
                    if (m_player) m_player->setPosition(0);
                } else {
                    long long t_prev = frames8x8[currentFrameIndex - 1].timestamp_ns;
                    long long t_curr = frames8x8[currentFrameIndex].timestamp_ns;
                    cloudTimeMs = (t_curr - t_prev) / 1000000;

                    if (m_player) {
                        m_player->setPosition(m_player->position() + cloudTimeMs);
                    }
                }

                int delay_ms = 1;
                if (currentFrameIndex < frames8x8.size() - 1) {
                    long long t_curr = frames8x8[currentFrameIndex].timestamp_ns;
                    long long t_next = frames8x8[currentFrameIndex + 1].timestamp_ns;
                    delay_ms = std::max(static_cast<int>((t_next - t_curr) / 1000000), 1);
                }

                if (!CarData.empty() && !CarSRSYawData.empty()) {
                    updateCarDataDisplay(frame.timestamp_ns);
                }

                currentFrameIndex++;
                playbackTimer->start(delay_ms);
            }

        } else {
            // =========================================================================
            // 非 8x8 模式 (4x4 历史旧模式，完全保留原汁原味)
            // =========================================================================
            if (currentFrameIndex >= multiFrames.size()-1) {
                playbackTimer->stop();
                currentFrameIndex = 0;
                isPlaying = false;
                ui->playPauseButton->setText("Play");
                if (m_player) m_player->stop();
                return;
            }

            currentFrameIndex++;
            satecloud = multiFrames[currentFrameIndex];

            isProgrammaticallyChangingSlider = true;
            ui->frameSlider->setValue(currentFrameIndex);
            isProgrammaticallyChangingSlider = false;

            ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frames[currentFrameIndex].start_time_ns+frames[currentFrameIndex].timestamp_ns)));
            ui->pointruntime->setText(QString::number(frames[currentFrameIndex].indexno ));

            updateCarDataDisplay(frames[currentFrameIndex].start_time_ns+frames[currentFrameIndex].timestamp_ns);
            displayPointCloud();

            if (isPlaying && m_player) {
                qint64 cloudTimeMs = static_cast<qint64>(frames[currentFrameIndex].timestamp_ns / 1000000);
                m_player->setPosition(cloudTimeMs);
            }

            long long current_time_ns = frames[currentFrameIndex].timestamp_ns;
            long long next_time_ns = frames[currentFrameIndex + 1].timestamp_ns;
            long long diff_ns = next_time_ns - current_time_ns;
            int delay_ms = static_cast<int>(diff_ns / 1000000);
            if (delay_ms < 1) {
                delay_ms = 1;
            }

            playbackTimer->start(delay_ms);
        }
    });
    // 初始化模拟雷达点云
    //RadarSimulator simulator;

    // 初始化定时器，每50ms刷新一次点云
    timer = new QTimer(this);
    //connect(timer, &QTimer::timeout, this, &MainWindow::updatePointCloud);
    //timer->start(50);




    ui->infoPanel->setParent(ui->qvtkWidget);    // 成为 qvtkWidget 的子控件
    // 1. 创建布局
    if (ui->infoPanel->layout()) delete ui->infoPanel->layout();
    QGridLayout* layout = new QGridLayout(ui->infoPanel);

    // 2. 设置内边距（Padding）：让文字离开红色边缘
    layout->setContentsMargins(5, 5, 5, 5);

    // 3. 设置垂直行间距
    layout->setVerticalSpacing(20);

    // 4. 加入控件
    layout->addWidget(ui->speedLabel, 0, 0, Qt::AlignLeft);
        layout->addWidget(ui->YawRate, 1, 0, Qt::AlignLeft);

    // 5. 🌟 修复：解除高度固定限制
    ui->infoPanel->setMinimumWidth(300);     // 设置一个最小宽高
    ui->infoPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    // ui->infoPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred); // 允许跟随布局伸缩
    ui->infoPanel->setAttribute(Qt::WA_StyledBackground);
    // 6. 强制刷新大小
    ui->infoPanel->adjustSize();

    ui->infoPanel->setFixedSize(200, 100);      // 固定宽高，也可以使用 adjustSize()
    ui->infoPanel->raise();                     // 确保在最上层
    ui->infoPanel->setAttribute(Qt::WA_TransparentForMouseEvents); // 点击穿透


    // 样式
    ui->infoPanel->setStyleSheet("background-color: rgba(255, 0, 0, 100);"); // 暂时用半透明红色，方便看位置
    ui->speedLabel->setStyleSheet("color: black; font-size: 10px;");
    ui->YawRate->setStyleSheet("color: black; font-size: 10px;");
    ui->speedLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);



    initInfoPanelStyle();
    ui->speedLabel->setText(
        "<html>"
        "<div><span style='font-size:14px; color:#FFD700;'>SPD </span>"
        "<span style='font-size:26px; color:#00FFCC; font-weight:bold;' >-.-</span>"
        "<span style='font-size:14px; color:#00FFCC;'> km/h</span></div>"
        "</html>"
    );
    ui->YawRate->setText(
        "<html>"
        "<span style='font-size:14px; color:#FFD700;'>YAW </span>"
        "<span style='font-size:26px; color:#00FFCC; font-weight:bold;'>-.-</span>"
        "<span style='font-size:14px; color:#00FFCC;'> deg/s</span>"
        "</html>"
    );
    updateInfoPanelPosition();

    ui->infoPanel->show();
    ui->infoPanel->raise();
    /*

    connect(ui->rangeslider,SIGNAL(lowerValueChanged(int)),this,SLOT(lowerValueChangedSlot(int)));
    connect(ui->rangeslider,SIGNAL(upperValueChanged(int)),this,SLOT(upperValueChangedSlot(int)));
    connect(ui->rangemin,SIGNAL(textChanged(QString)),this,SLOT(lowerTextChangedSlot(QString)));
    connect(ui->rangemax,SIGNAL(textChanged(QString)),this,SLOT(upperTextChangedSlot(QString)));

    connect(ui->dopplerSpeedslider,SIGNAL(lowerValueChanged(int)),this,SLOT(lowerValueChangedSlot(int)));
    connect(ui->dopplerSpeedslider,SIGNAL(upperValueChanged(int)),this,SLOT(upperValueChangedSlot(int)));
    connect(ui->dopplerSpeedmin,SIGNAL(textChanged(QString)),this,SLOT(lowerTextChangedSlot(QString)));
    connect(ui->dopplerSpeedmax,SIGNAL(textChanged(QString)),this,SLOT(upperTextChangedSlot(QString)));

    */
    setupSpanSliders();
    pointminmaxData =nullptr;
    initSlidersFromPointCloud(pointminmaxData);
    connect(ui->resetFilterBtn, &QPushButton::clicked,
            this, &MainWindow::onResetFilterClicked);

    connect(ui->applyFilterBtn, &QPushButton::clicked,
            this, &MainWindow::onApplyFilterClicked);


    connect(ui->Xrangevalue, SIGNAL(valueChanged(int)),
            this, SLOT(setUserXRange(int)));

    connect(ui->Yrangevalue, SIGNAL(valueChanged(int)),
            this, SLOT(setUserYRange(int)));


    connect(ui->FOVMIN, SIGNAL(valueChanged(int)),
            this, SLOT(setSectorStartAngle(int)));

    // 2. 连接结束角度 SpinBox
    connect(ui->FOVMAX, SIGNAL(valueChanged(int)),
            this, SLOT(setSectorEndAngle(int)));

    // 3. 初始化 SpinBox 的默认值 (确保 UI 与内部成员变量同步)
    // 假设您的 spinBox 对象是存在的
    if (ui->FOVMIN) {
        ui->FOVMIN->setValue(static_cast<int>(sectorStartAngle));
    }
    if (ui->FOVMAX) {
        ui->FOVMAX->setValue(static_cast<int>(sectorEndAngle));
    }


    // 假设您的颜色选择下拉框是 ui->colorModeComboBox
    connect(ui->colorModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onColorModeChanged);


    if (ui->playframecount) { // 假设您的 QSpinBox 叫 spinBoxFrames
        connect(ui->playframecount, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::setMaxFramesToAccumulate);


        // 首次初始化时，确保成员变量获取初始值
        maxFramesToAccumulate = ui->playframecount->value();
    }

    // 确保 SpinBox 的初始值与 userXRange/userYRange 的默认值一致
    ui->Xrangevalue->setValue(static_cast<int>(userXRange));
    ui->Yrangevalue->setValue(static_cast<int>(userYRange));

    ui->PointFilters->setAllowedAreas(Qt::AllDockWidgetAreas);
    ui->RD->setAllowedAreas(Qt::AllDockWidgetAreas);
    ui->Video->setAllowedAreas(Qt::AllDockWidgetAreas);

    // QImage rdMapImage=dspdll_Test();
    // ui->RDMap->setPixmap(QPixmap::fromImage(rdMapImage));
    // ui->RDMap->setScaledContents(true);


    rdMapPlot = new QCustomPlot(ui->dockWidgetContents_2); // <-- 将父对象设置为 dockWidgetContents_2
    // const int MAX_WIDTH = 400;
    // const int MAX_HEIGHT = 300;
    // rdMapPlot->setMaximumSize(MAX_WIDTH, MAX_HEIGHT);
    rdMapPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QLayout *existingLayout = ui->dockWidgetContents_2->layout();
    if (existingLayout) {
        existingLayout->addWidget(rdMapPlot);
        existingLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        // 容错：如果 Designer 中没有布局，手动创建一个
        QVBoxLayout *newLayout = new QVBoxLayout(ui->dockWidgetContents_2);
        newLayout->addWidget(rdMapPlot);
        newLayout->setContentsMargins(0, 0, 0, 0);
    }


    // 连接 ComboBox 信号
    connect(ui->ViewupcomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::on_ViewupcomboBox_currentIndexChanged);

    // 确保在连接后，初始值被应用
    // 假设您在 Designer 中设置了默认索引 0，这里触发一次确保 ViewUp 初始设置正确
    on_ViewupcomboBox_currentIndexChanged(ui->ViewupcomboBox->currentIndex());


    connect(ui->XYazimuspinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onXYAzimuthChanged);

    // QCustomPlot 默认支持交互，但你的代码已经设置了，保持不变
    // rdMapPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    // setupRangeDopplerPlotnew(rdMapPlot);

    // QWidget *placeholderWidget = ui->dockWidgetContents_2;
    // if (!placeholderWidget) {
    //     qDebug() << "Error: dockWidgetContents_2 placeholder not found!";
    //     return;
    // }

    // // 1. 创建 Q3DSurface 实例
    // Q3DSurface *graph = new Q3DSurface();
    // setupRangeDopplerPlot3D(graph);

    // // 2. 使用 QWidget::createWindowContainer 封装 Q3DSurface
    // QWidget *container = QWidget::createWindowContainer(graph, placeholderWidget);

    // // 关键修正 1：强制容器拉伸策略
    // // 告诉布局管理器：这个容器应该占用所有可用空间
    // container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    // // 3. 检查并设置/获取布局
    // QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(placeholderWidget->layout());

    // // 关键修正 2：如果内容区域没有布局，则创建一个 QVBoxLayout
    // if (layout) {
    //     // 关键 3: 将容器添加到现有的布局中
    //     layout->addWidget(container);
    //     layout->setContentsMargins(0, 0, 0, 0);
    //     // 不需要再次调用 placeholderWidget->setLayout(layout);
    // } else {
    //     qDebug() << "Error: Layout 'verticalLayout_3' not found in dockWidgetContents_2!";
    // }

    // // 5. 优化：移除布局边距
    // layout->setContentsMargins(0, 0, 0, 0);

    // // -----------------------------------------------------------
    // // 注意：如果布局是新建的，setLayout(layout) 已经在 if 块中完成。
    // // 如果布局是获取的，则不需要再次调用 setLayout。
    // // -----------------------------------------------------------

    // // 确保在 graph 设置完成后，它的父窗口可以被重绘
    // placeholderWidget->update();

    // 1. 设置组织名称（最好放在 main.cpp，放这里也可以）
    // setupApplicationSettings();

    // 2. 只有在没有保存过状态的情况下，才执行这个默认比例
    // 如果 restoreState 成功，它会覆盖这个比例

    // 3. 加载设置（包含 restoreState）
    // 3. 检查是否有历史记录



    m_processor = new PcapProcessor(this);
    m_parser = new RadarDataParser(this);

    // connect(m_processor, &PcapProcessor::radarFrameParsed,
    //             m_parser, &RadarDataParser::parseFrame);

    connect(m_processor, &PcapProcessor::radarFrameParsed, this, [this](quint32 frameId, const QByteArray &data) {
        m_parser->parseFrame(frameId, data);
    });


    // 2. 🌟 新的信号连接：只要是它发出来的，100% 是离线不连续 BIN 文件
    connect(m_processor, &PcapProcessor::binFrameParsed, this, [this](quint32 frameId, const QByteArray &data) {
        // 稳稳地走咱们刚才写好的 3 参数离线完美对齐版本
        m_parser->parseFrame(frameId, data, true);
    });

    connect(m_parser, &RadarDataParser::frameReady, this, &MainWindow::onNewFrameReady);

    connect(m_processor, &PcapProcessor::binFolderLoadingFinished,
            this, &MainWindow::onBinFolderLoadingFinished);

    // Start 信号处理
    connect(onlineDlg, &RadarOnlineDialog::startCapture, this, [=](QString dev, QString ip, int port){
        isFirstRealtimeFrame = true;
        isFirstFrame = true;

        // 清空所有智能指针指向的内容
        if (satecloud) satecloud->clear();
        if (cumulativeCloud) cumulativeCloud->clear();

        // 如果上一次停止时还有 Actor 残留，强制移除
        if (pointCloudActor) {
            mainRenderer->RemoveActor(pointCloudActor);
            pointCloudActor = nullptr;
        }

        m_radarMgr->start(dev, ip, port);
    });


    // Stop 信号处理
    connect(onlineDlg, &RadarOnlineDialog::stopCapture, this, [=](){
        m_radarMgr->stop();
        isFirstRealtimeFrame = true; // 确保下次 Start 走初始化

        // 停止后立刻把画面清空
        if (satecloud) satecloud->clear();
        displayPointCloud(); // 调用此函数会进入空云处理逻辑，移除 Actor
    });


    // 初始化过滤点云指针
        filteredCloudPtr = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();
        cumulativeCloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();

        // // 预创建色带（以速度/多普勒为例）
        // radarLUT = vtkSmartPointer<vtkLookupTable>::New();
        // radarLUT->SetNumberOfTableValues(256);
        // radarLUT->SetHueRange(0.667, 0.0); // 从蓝到红
        // radarLUT->Build();
        // isFirstFrame = true;


        // 2. 初始化实时管理器
        m_radarMgr = new RadarCaptureManager(this);
        // 3. 关键：连接实时信号到渲染槽函数
        // 1. 注册类型（跨线程必须）
        qRegisterMetaType<pcl::PointCloud<PointXYZRGBWithProperties>::Ptr>("pcl::PointCloud<PointXYZRGBWithProperties>::Ptr");

        // 2. 连接信号
        bool ok = connect(m_radarMgr, &RadarCaptureManager::cloudReady, this, &MainWindow::onRealtimeCloudReady);
        qDebug() << "Signal connection status:" << ok;

    QSettings settings;
    if (settings.value("mainWindow/windowState").isNull()) {
        // 【仅第一次运行】执行默认布局逻辑
        addDockWidget(Qt::LeftDockWidgetArea, ui->ControlDock);
            addDockWidget(Qt::LeftDockWidgetArea, ui->PointCloudDock);

            addDockWidget(Qt::RightDockWidgetArea, ui->Video);
            addDockWidget(Qt::RightDockWidgetArea, ui->RD);
            addDockWidget(Qt::RightDockWidgetArea, ui->PointFilters);
            addDockWidget(Qt::RightDockWidgetArea, loadedFilesDock);



            splitDockWidget(ui->ControlDock, ui->PointCloudDock, Qt::Horizontal);

            splitDockWidget(ui->PointCloudDock, ui->Video, Qt::Horizontal);
            splitDockWidget(ui->Video, ui->RD, Qt::Vertical);
            splitDockWidget(ui->RD, ui->PointFilters, Qt::Vertical);
            splitDockWidget(ui->PointFilters, loadedFilesDock, Qt::Vertical);
            addDockWidget(Qt::BottomDockWidgetArea, ui->PointData);
            splitDockWidget(ui->PointCloudDock, ui->PointData, Qt::Vertical);
            resizeDocks({ ui->PointData }, { 80 }, Qt::Vertical);
    } else {
        // 【非第一次运行】直接加载
        loadSettings();
    }

    // 4. 最后显示窗口
    // 注意：不要在 loadSettings 之前 showMaximized
    this->showMaximized();

    // 5. 【关键技巧】如果 PointData 依然太高，在这里补一刀
    // 强制让布局引擎重新根据当前的 SizePolicy 挤压一次


    // 构造函数


    // 初始状态（全部 N/A）
    loadedFilesDock->updatePointBin("");
    loadedFilesDock->updateVideo("");
    loadedFilesDock->updateCarDataMF4("");
    loadedFilesDock->updateRadarMF4("");
    loadedFilesDock->updateADCBin("");
    loadedFilesDock->updateOnePCD("");




}


void MainWindow::initInfoPanelStyle() {
    // 面板样式
    ui->infoPanel->setStyleSheet(
        "QWidget#infoPanel {"
        "  background-color: rgba(25, 25, 25, 190);" // 稍微加深，更显沉稳
        "  border: 1px solid rgba(0, 255, 204, 150);" // 边框变细，增加半透明度
        "  border-radius: 10px;"
        "}"
    );

    // 字体大变身：使用更为精炼的字体
    // 推荐顺序：Segoe UI (Windows现代字体) > Montserrat > Microsoft YaHei (微软雅黑)
    QString labelStyle =
        "QLabel {"
        "  color: #00FFCC;"
        "  font-family: 'Segoe UI Semibold', 'Microsoft YaHei', sans-serif;"
        "  font-size: 16px;"
        "  letter-spacing: 1px;" // 增加字符间距，更有高端感
        "  background: transparent;"
        "}";

    ui->speedLabel->setStyleSheet(labelStyle);
    ui->YawRate->setStyleSheet(labelStyle);
}


void MainWindow::updatePointCloudFromVideo(qint64 videoMs)
{
    if (isRadar8x8Mode && frames8x8.empty()) return; // 没有点云就直接返回
    if (!isRadar8x8Mode && multiFrames.empty()) return;
    if (frames8x8.empty() && frames.empty()) return;

    long long videoTimeNs = videoMs * 1000000LL;

    int frameIdx = 0;
    if (isRadar8x8Mode) {
        for (size_t i = 0; i < frames8x8.size(); ++i) {
            long long frameTimeNs = frames8x8[i].timestamp_ns - frames8x8[0].timestamp_ns;
            if (frameTimeNs > videoTimeNs) {
                frameIdx = static_cast<int>(i > 0 ? i - 1 : 0);
                break;
            }
            frameIdx = static_cast<int>(i);
        }
        currentFrameIndex = frameIdx;
        auto &frame = frames8x8[currentFrameIndex];
        satecloud = frame.Radar8T8Point.cloud;
        displayPointCloud();
        showRDMap(rdMapPlot, frame.Radar8T8Point.rdmap.data());
        ui->pointruntime->setText(QString::number(frame.indexno));
        ui->pointstarttime->setText(QString::fromStdString(
            convertTimestamp(frame.start_time_ns + frame.timestamp_ns)));
        ui->frameSlider->setValue(currentFrameIndex); // 更新点云 slider
    } else {
        for (size_t i = 0; i < frames.size(); ++i) {
            long long frameTimeNs = frames[i].timestamp_ns;
            if (frameTimeNs > videoTimeNs) {
                frameIdx = static_cast<int>(i > 0 ? i - 1 : 0);
                break;
            }
            frameIdx = static_cast<int>(i);
        }
        currentFrameIndex = frameIdx;
        satecloud = multiFrames[currentFrameIndex];
        displayPointCloud();
        ui->pointruntime->setText(QString::number(frames[currentFrameIndex].indexno));
        ui->pointstarttime->setText(QString::fromStdString(
            convertTimestamp(frames[currentFrameIndex].start_time_ns + frames[currentFrameIndex].timestamp_ns)));
        ui->frameSlider->setValue(currentFrameIndex);
    }
}

void MainWindow::onXYAzimuthChanged(int value)
{
    vtkCamera* camera = mainRenderer->GetActiveCamera();
       if (!camera) return;

       camera->DeepCopy(baseCamera);


       // 🌟 关键：绕 Y 轴旋转
       camera->Elevation(value);

       // 固定你的坐标系定义
       camera->SetViewUp(-1, 0, 0);
       camera->OrthogonalizeViewUp();
       camera->SetWindowCenter(0, 0.8);

       mainRenderer->ResetCameraClippingRange();
       ui->qvtkWidget->renderWindow()->Render();

      int index = ui->ViewupcomboBox->currentIndex();
      on_ViewupcomboBox_currentIndexChanged(index);
}



void MainWindow::on_ViewupcomboBox_currentIndexChanged(int index)
{
    // 确保 currentCamera 是 mainRenderer->GetActiveCamera() 的引用或指针
    vtkCamera* camera = mainRenderer ? mainRenderer->GetActiveCamera() : nullptr;

    if (!camera || index < 0) return;

    double x = 0.0, y = 0.0, z = 0.0;

    // 假设索引顺序如下（与上一个回复一致）：
    switch (index) {
    case 0: //  -X 向上 (-1, 0, 0)
        x = -1.0;
        break;
    case 1: //  +X 向上 (1, 0, 0)

        x = 1.0;
        break;
    case 2: // +Y 向上 (0, 1, 0)
        y = -1.0;
        break;
    case 3: // -Y 向上 (0, -1, 0)
        y = 1.0;
        break;
    default:
        // qDebug() << "Warning: Unknown ViewUp index selected.";
        return;
    }

    // qDebug() << "ViewUp ComboBox changed. Setting ViewUp to:" << x << y << z;

    // 核心步骤：应用 ViewUp
    camera->SetViewUp(x, y, z);

    // 核心增强：确保 ViewUp 垂直于视线（模仿 setupBackgroud 中的调用）
    camera->OrthogonalizeViewUp();
    AddGridAndLabelsDynamically();
    // 强制 VTK 窗口重新渲染
    ui->qvtkWidget->renderWindow()->Render();
}

void MainWindow::onColorModeChanged(int index)
{
    // 1. 更新颜色模式
    ColorMode newMode;
    switch(index){
    case 0: newMode = ColorMode::RadialVelocity; break;
    case 1: newMode = ColorMode::RCS; break;
    case 2: newMode = ColorMode::Height; break;
    case 3: newMode = ColorMode::Fixed; break;
    case 4: newMode = ColorMode::SNR; break;
    default: return;
    }
    if (newMode == currentColorMode) return;
    currentColorMode = newMode;

    // // 2. 遍历所有已缓冲帧，重新上色
    // for (auto& frame : frameBuffer) {
    //     colorizer.colorizePointCloud(frame, currentColorMode);
    // }

    // // 3. 当前帧也重新上色
    // if (satecloud)
    //     colorizer.colorizePointCloud(satecloud, currentColorMode);

    // 4. 重新生成累积点云并刷新 PolyData
    colorUpdateOnly = true;
    displayPointCloud(); // 内部使用 frameBuffer 重新累积
    colorUpdateOnly = false;
}

QString MainWindow::formatTime(qint64 ms) {
    int totalSeconds = static_cast<int>(ms / 1000);
       int minutes = totalSeconds / 60;
       int hours   = minutes / 60;
       minutes    %= 60;

       int seconds_int = totalSeconds % 60;
       int milliseconds = ms % 1000;  // 毫秒部分

       return QString("%1:%2:%3.%4")
           .arg(hours,   2, 10, QChar('0'))
           .arg(minutes, 2, 10, QChar('0'))
           .arg(seconds_int, 2, 10, QChar('0'))
           .arg(milliseconds, 3, 10, QChar('0'));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onDisplayPointcloudChanged(int state)
{
    Q_UNUSED(state);
    updateLayout();

}

void MainWindow::onDisplayVideoChanged(int state)
{
    Q_UNUSED(state);
    updateLayout();
}


void MainWindow::updateLayout()
{
    /*bool showPoint = ui->DisplayPointcloud->isChecked();
    bool showVideo = ui->DisplayVideo->isChecked();
    bool pointfilter= ui->pointfilter->isChecked();


    // 至少一个必须显示，防止都不选
    if (!showPoint && !showVideo) {
        // 默认显示 PointCloud
        ui->DisplayPointcloud->setChecked(true);
        return;
    }

    // 设置控件可见性
    ui->pointpanel->setVisible(showPoint);
    ui->videopanel->setVisible(showVideo);


    ui->horizontalLayout->activate();


    // 设置伸缩比例
    if (showPoint && !showVideo) {
        // 只显示 pointpanel，点云面板占满全宽

        ui->horizontalLayout->setStretch(0, 0);
        ui->horizontalLayout->setStretch(1, 1);
        ui->horizontalLayout->setStretch(2, 0);
    } else if (!showPoint && showVideo) {
        // 只显示 videopanel，视频面板占满全宽

        ui->pointfilter->setCheckState(Qt::Unchecked);
        ui->horizontalLayout->setStretch(0, 0);
        ui->horizontalLayout->setStretch(1, 0);
        ui->horizontalLayout->setStretch(2, 1);
    } else {
        // 两个都显示，平均分配宽度

        ui->horizontalLayout->setStretch(0, 0);
        ui->horizontalLayout->setStretch(1, 1);
        ui->horizontalLayout->setStretch(2, 1);
    }

    */


    updateInfoPanelPosition();

}


void MainWindow::setupCameraWithOriginAtViewportBottom10Percent() {
    vtkCamera* camera = mainRenderer->GetActiveCamera();

    // 相机离焦点的水平距离（Y轴负方向）
    double distanceY = 600;

    // 相机焦点
    double focalPoint[3] = {0, 0, 0};

    // 视角（视野高度对应的俯仰角）
    double elevationAngleDeg = 20.0;
    double elevationAngleRad = vtkMath::RadiansFromDegrees(elevationAngleDeg);

    // 计算相机Z，使得原点投影到视口Y轴的10%位置
    // 视口高度视角 = 2 * distanceY * tan(FOV/2)
    // 这里用俯仰角elevationAngle代替FOV计算相机高度
    // 视口中心点对应视线，原点比中心低视口10% => offset
    double normalizedY = 0.1; // 原点想显示在视口底部10%

    // 计算视口高度
    // 假设FOV约为40度（可以通过相机GetViewAngle获取或设置）
    double fovDeg = camera->GetViewAngle();
    if (fovDeg < 1) fovDeg = 40; // 默认40度防止0值
    double fovRad = vtkMath::RadiansFromDegrees(fovDeg);

    double viewHeight = 2.0 * distanceY * tan(fovRad / 2);

    // 视口中心是0.5，原点想在0.1，差值0.4
    double offsetViewport = (0.5 - normalizedY) * viewHeight;

    // 根据俯仰角计算相机Z轴高度
    double cameraZ = offsetViewport / tan(elevationAngleRad);

    // 设置相机位置
    double cameraPos[3] = {0, -distanceY, cameraZ};

    camera->SetFocalPoint(focalPoint);
    camera->SetPosition(cameraPos);
    camera->SetViewUp(0, 0, 1);

    mainRenderer->ResetCameraClippingRange();
    ui->qvtkWidget->renderWindow()->Render();
}

/*
void MainWindow::setupBackgroud(){

    if (!mainRenderer) {
        mainRenderer = vtkSmartPointer<vtkRenderer>::New();
        ui->qvtkWidget->renderWindow()->AddRenderer(mainRenderer);
    }

    // 网格参数
    double sizeX = 300;
    double sizeY = 400;
    double spacing = 25;

    // 创建网格点和线
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // 横向线 正向for (double x = 0; x <= sizeX; x += spacing)
    for (double x = -sizeX; x <= 0; x += spacing) {
        vtkIdType p1 = points->InsertNextPoint(x, -sizeY/2, 0);
        vtkIdType p2 = points->InsertNextPoint(x, sizeY/2, 0);
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, p1);
        line->GetPointIds()->SetId(1, p2);
        lines->InsertNextCell(line);
    }

    // 纵向线
    for (double y = -sizeY/2; y <= sizeY/2; y += spacing) {
        vtkIdType p1 = points->InsertNextPoint(0, y, 0);
        vtkIdType p2 = points->InsertNextPoint(-sizeX, y, 0);// 正的X方向(sizeX, y, 0)
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, p1);
        line->GetPointIds()->SetId(1, p2);
        lines->InsertNextCell(line);
    }

    vtkSmartPointer<vtkPolyData> grid = vtkSmartPointer<vtkPolyData>::New();
    grid->SetPoints(points);
    grid->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(grid);
    //vtkSmartPointer<vtkActor> gridActor = vtkSmartPointer<vtkActor>::New();
    gridActor = vtkSmartPointer<vtkActor>::New();
    gridActor->SetMapper(mapper);
    gridActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    gridActor->GetProperty()->SetLineWidth(0.5);
    mainRenderer->AddActor(gridActor);

    // 坐标轴
    dynamicAxesActor = vtkSmartPointer<vtkAxesActor>::New();
    dynamicAxesActor->SetTotalLength(50, 50, 50);
    dynamicAxesActor->SetShaftTypeToCylinder();
    dynamicAxesActor->SetCylinderRadius(0.005);
    dynamicAxesActor->SetAxisLabels(true);

    dynamicAxesActor->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    dynamicAxesActor->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    dynamicAxesActor->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();

    dynamicAxesActor->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    dynamicAxesActor->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);
    dynamicAxesActor->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetFontSize(12);

    dynamicAxesActor->GetXAxisShaftProperty()->SetLineWidth(1.0);
    dynamicAxesActor->GetYAxisShaftProperty()->SetLineWidth(1.0);
    dynamicAxesActor->GetZAxisShaftProperty()->SetLineWidth(1.0);

    dynamicAxesActor->GetXAxisShaftProperty()->SetColor(0.5, 0.5, 0.5);
    dynamicAxesActor->GetYAxisShaftProperty()->SetColor(0.5, 0.5, 0.5);
    dynamicAxesActor->GetZAxisShaftProperty()->SetColor(0.5, 0.5, 0.5);

    dynamicAxesActor->SetPosition(0, 0, 0);
    mainRenderer->AddActor(dynamicAxesActor);

    // 背景渐变色
    mainRenderer->GradientBackgroundOn();
    mainRenderer->SetBackground(1.0,1.0, 1.0);
    mainRenderer->SetBackground2(1.0,1.0, 1.0);

    //mainRenderer->SetBackground(0.5, 0.5, 0.5);
    //mainRenderer->SetBackground2(0.5, 0.5, 0.5)

    // 相机设置
    vtkCamera* camera = mainRenderer->GetActiveCamera();
    double distance = 650;
    double tiltAngle = 180; // 绕Y轴俯视角度
    // 变更前 120  x_new camera->SetViewUp(0, 0, 1)
    // 变更后 30  -x_new camera->SetViewUp(0, 0, -1)
    double pos[3] = {0, 0, -distance};
    double rad = vtkMath::RadiansFromDegrees(tiltAngle);
    double x_new = pos[0] * cos(rad) + pos[2] * sin(rad);
    double z_new = -pos[0] * sin(rad) + pos[2] * cos(rad);

    camera->SetPosition(-x_new, pos[1], z_new);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(0, 0, 1);
    camera->OrthogonalizeViewUp();

    // 将原点显示在视口下方10%
    double offsetY = 0.8; // 负值向下偏
    camera->SetWindowCenter(0, offsetY);

    mainRenderer->ResetCameraClippingRange();
    ui->qvtkWidget->renderWindow()->Render();


    AddYlabel( sizeX,sizeY);
    AddSector(0, 0, 0, 200, 120, 240, 50);
}

*/

void MainWindow::setupBackgroud()
{
    // 1. 渲染器和基本初始化
    if (!mainRenderer) {
        mainRenderer = vtkSmartPointer<vtkRenderer>::New();
        ui->qvtkWidget->renderWindow()->AddRenderer(mainRenderer);
    }


    // 2. 初始化动态网格的核心数据结构
    // 避免崩溃的关键：确保所有智能指针在使用 Reset() 或 SetInputData() 前被实例化。
    if (!gridPoints) {
        gridPoints = vtkSmartPointer<vtkPoints>::New();
        gridLines = vtkSmartPointer<vtkCellArray>::New();
        gridPolyData = vtkSmartPointer<vtkPolyData>::New();

        gridPolyData->SetPoints(gridPoints);
        gridPolyData->SetLines(gridLines);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(gridPolyData);

        gridActor = vtkSmartPointer<vtkActor>::New();
        gridActor->SetMapper(mapper);
        gridActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
        gridActor->GetProperty()->SetLineWidth(0.5);
        gridActor->SetPickable(false);
        mainRenderer->AddActor(gridActor);
    }

    if (!tickActor) {
        // 创建局部智能指针
        vtkSmartPointer<vtkPoints> localTickPoints = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> localTickLines = vtkSmartPointer<vtkCellArray>::New();

        // 赋值给成员变量
        this->tickPoints = localTickPoints; // 赋值给新声明的成员
        this->tickLines = localTickLines;   // 赋值给新声明的成员

        vtkSmartPointer<vtkPolyData> tickPolyData = vtkSmartPointer<vtkPolyData>::New();

        // 使用成员变量设置 PolyData
        tickPolyData->SetPoints(this->tickPoints);
        tickPolyData->SetLines(this->tickLines);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(tickPolyData);

        tickActor = vtkSmartPointer<vtkActor>::New();
        tickActor->SetMapper(mapper);
        tickActor->GetProperty()->SetColor(0.3, 0.3, 0.3); // 黑色
        tickActor->GetProperty()->SetLineWidth(1.5);
        tickActor->SetPickable(false);

        mainRenderer->AddActor(tickActor);
    }


    if (!xAxisActor) {
        // 创建几何数据结构：点、单元格 (PolyLine)
        vtkSmartPointer<vtkPoints> axisPoints = vtkSmartPointer<vtkPoints>::New();
        axisPoints->SetNumberOfPoints(2); // X 轴只有两个端点

        vtkSmartPointer<vtkPolyLine> axisLine = vtkSmartPointer<vtkPolyLine>::New();
        axisLine->GetPointIds()->SetNumberOfIds(2);
        axisLine->GetPointIds()->SetId(0, 0); // 引用 axisPoints 的第一个点
        axisLine->GetPointIds()->SetId(1, 1); // 引用 axisPoints 的第二个点

        vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
        cells->InsertNextCell(axisLine); // 将 PolyLine 放入 CellArray

        vtkSmartPointer<vtkPolyData> axisPolyData = vtkSmartPointer<vtkPolyData>::New();
        axisPolyData->SetPoints(axisPoints);
        axisPolyData->SetLines(cells); // 设置线的单元格

        // 创建 Mapper 和 Actor
        vtkSmartPointer<vtkPolyDataMapper> axisMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        axisMapper->SetInputData(axisPolyData);

        xAxisActor = vtkSmartPointer<vtkActor>::New();
        xAxisActor->SetMapper(axisMapper);
        // 初始属性设置
        xAxisActor->GetProperty()->SetColor(0.0, 0.0, 0.0); // 黑色
        xAxisActor->GetProperty()->SetLineWidth(2.0);        // 粗线
        xAxisActor->SetPickable(false);

        mainRenderer->AddActor(xAxisActor);
    }

    // Y 轴 Actor 初始化
    if (!yAxisActor) {
        // 创建几何数据结构：点、单元格 (PolyLine)
        vtkSmartPointer<vtkPoints> axisPoints = vtkSmartPointer<vtkPoints>::New();
        axisPoints->SetNumberOfPoints(2); // Y 轴只有两个端点

        vtkSmartPointer<vtkPolyLine> axisLine = vtkSmartPointer<vtkPolyLine>::New();
        axisLine->GetPointIds()->SetNumberOfIds(2);
        axisLine->GetPointIds()->SetId(0, 0);
        axisLine->GetPointIds()->SetId(1, 1);

        vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
        cells->InsertNextCell(axisLine);

        vtkSmartPointer<vtkPolyData> axisPolyData = vtkSmartPointer<vtkPolyData>::New();
        axisPolyData->SetPoints(axisPoints);
        axisPolyData->SetLines(cells);

        // 创建 Mapper 和 Actor
        vtkSmartPointer<vtkPolyDataMapper> axisMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        axisMapper->SetInputData(axisPolyData);

        yAxisActor = vtkSmartPointer<vtkActor>::New();
        yAxisActor->SetMapper(axisMapper);
        // 初始属性设置
        yAxisActor->GetProperty()->SetColor(0.0, 0.0, 0.0); // 黑色
        yAxisActor->GetProperty()->SetLineWidth(2.0);        // 粗线
        yAxisActor->SetPickable(false);

        mainRenderer->AddActor(yAxisActor);
    }
    // -----------------------------
    // 原点十字 Actor（一次性创建）
    // -----------------------------
    if (!originActor) {
        originPoints = vtkSmartPointer<vtkPoints>::New();
        originLines  = vtkSmartPointer<vtkCellArray>::New();
        originPolyData = vtkSmartPointer<vtkPolyData>::New();

        originPolyData->SetPoints(originPoints);
        originPolyData->SetLines(originLines);

        vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputData(originPolyData);

        originActor = vtkSmartPointer<vtkActor>::New();
        originActor->SetMapper(mapper);
        originActor->GetProperty()->SetColor(0.0, 0.0, 0.0); // 黑色
        originActor->GetProperty()->SetLineWidth(3.0);       // 粗线
        originActor->SetPickable(false);

        mainRenderer->AddActor(originActor);
    }


    // 4. 背景和相机设置

    // 背景颜色 (模仿 MATLAB 的浅灰色背景)
    mainRenderer->GradientBackgroundOff();
    mainRenderer->SetBackground(0.96, 0.96, 0.96); // 浅灰

    vtkCamera* camera = mainRenderer->GetActiveCamera();

    // 🌟 启用正交投影 (2D 关键)
    camera->SetParallelProjection(true);

    // 初始位置：从 Z 轴正向看，X 轴向右，Y 轴向上 (标准 2D 视图)
    camera->SetPosition(0, 0, 600);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(-1, 0, 0);
    camera->OrthogonalizeViewUp();

    // 设置初始缩放（控制视口高度的一半）
    double initialParallelScale = userXRange>userYRange?userXRange:userYRange;
    camera->SetParallelScale(initialParallelScale);

    baseCamera = vtkSmartPointer<vtkCamera>::New();
    baseCamera->DeepCopy(mainRenderer->GetActiveCamera());

    /* // 将焦点/原点放在视口中央（如果需要偏移，请调整 SetWindowCenter）
    // camera->SetWindowCenter(0, 0);
    double shiftFactor = 0.8;
    double cx_offset = 0.0;
    double cy_offset = shiftFactor; // 向上偏移画面，让 X=0 区域可见
    camera->SetWindowCenter(cx_offset, cy_offset);

    mainRenderer->ResetCameraClippingRange();
    */
    // 5. 首次调用动态更新逻辑
    // 这将首次绘制网格和标签，并根据初始 ParallelScale 确定间距。
    // AddGridAndLabelsDynamically();

    // 6. 连接交互事件
    vtkRenderWindowInteractor *interactor = ui->qvtkWidget->renderWindow()->GetInteractor();
    if (interactor) {
        vtkSmartPointer<vtkCallbackCommand> callback = vtkSmartPointer<vtkCallbackCommand>::New();
        callback->SetClientData(this);

        // 确保 OnInteractionCallback 是 MainWindow 的静态成员
        callback->SetCallback(MainWindow::OnInteractionCallback);

        // 在交互结束（平移、缩放）时触发更新
        interactor->AddObserver(vtkCommand::EndInteractionEvent, callback);
    }

    // 7. 使用 QTimer 延迟调用初始化逻辑 (关键修复)
    // 延迟 50 毫秒（或 100 毫秒）执行，确保 Qt 窗口和 QVTKWidget 完成初始化。
    QTimer::singleShot(50, this, SLOT(delayedGridInitialization()));

    // 8. 您的扇形绘制 (如果扇形也依赖网格，请移入 delayedGridInitialization)
    // 如果扇形不需要网格数据，可以保留。如果它出现在网格之后，也请移动。
    // 为了安全，也将其移动。
    // AddSector(0, 0, 0, 200, 120, 240, 50);
    // 7. 渲染
    //ui->qvtkWidget->renderWindow()->Render();

    // 您的扇形绘制
    //AddSector(0, 0, 0, 200, 120, 240, 50);


    createSectorGeometry();

}




void MainWindow::delayedGridInitialization()
{
    if (!mainRenderer) return;
    vtkCamera* camera = mainRenderer->GetActiveCamera();
    if (!camera) return;

    double savedViewUp[3];
    camera->GetViewUp(savedViewUp);

    // 固定初始缩放和焦点
    double initialParallelScale = userXRange > userYRange ? userXRange : userYRange;
    camera->SetPosition(0, 0, 600);
    camera->SetFocalPoint(0, 0, 0);
    camera->SetViewUp(savedViewUp[0], savedViewUp[1], savedViewUp[2]);
    camera->OrthogonalizeViewUp();
    camera->SetParallelProjection(true);
    camera->SetParallelScale(initialParallelScale);
    camera->SetWindowCenter(0.0, 0.8);

    // 绘制网格 + 刻度 + 原点十字
    AddGridAndLabelsDynamically();

    QTimer::singleShot(50, this, [this](){
           AddGridAndLabelsDynamically();
       });

    // 重置剪切范围
    mainRenderer->ResetCameraClippingRange();
    ui->qvtkWidget->renderWindow()->Render();
}

// 静态回调函数，用于将 VTK 事件转发到成员函数
void MainWindow::OnInteractionCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
    MainWindow* self = static_cast<MainWindow*>(clientData);
    if (self) {
        self->AddGridAndLabelsDynamically();
    }
}

// void MainWindow::OnInteractionCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
// {
//     MainWindow* self = static_cast<MainWindow*>(clientData);
//     if (!self || !self->mainRenderer) return;

//     vtkCamera* camera = self->mainRenderer->GetActiveCamera();
//     if (!camera) return;

//     // 更新坐标轴摄像机绑定
//     if (self->cubeAxesActor)
//     {
//         self->cubeAxesActor->SetCamera(camera);

//         // 更新范围，可选：让坐标轴范围跟随缩放更新
//         double bounds[6];
//         self->mainRenderer->ComputeVisiblePropBounds(bounds);
//         bounds[4] = bounds[5] = 0.0; // 仅 XY 平面
//         self->cubeAxesActor->SetBounds(bounds);
//         self->cubeAxesActor->Modified();
//     }

//     self->ui->qvtkWidget->renderWindow()->Render();
// }


// // 核心动态更新逻辑
// void MainWindow::AddGridAndLabelsDynamically()
// {
//     if (!mainRenderer) return;

//     vtkCamera* camera = mainRenderer->GetActiveCamera();
//     double parallelScale = camera->GetParallelScale();

//     // 1. 计算新的理想网格间距 (Spacing)
//     // ... (Block 1 保持不变) ...
//     double viewHeight = parallelScale * 2;
//     double idealSpacing = viewHeight / 8.0;
//     double spacing = findNiceNumberSpacing(idealSpacing);

//     if (parallelScale >= 180.0 && spacing != 25.0) {
//         spacing = 50.0;
//     }else if (parallelScale >= 150.0 && spacing != 25.0){
//         spacing = 10.0;
//     }
//     else if (spacing < 0.1) {
//         spacing = 0.1;
//     }
//     currentGridSpacing = spacing;

//     // ----------------------------------------------------
//     // 2. 计算网格的世界坐标边界 (基于视口中心和缩放)
//     // ----------------------------------------------------
//     double center[3];
//     camera->GetFocalPoint(center);
//     double halfWidth = parallelScale * mainRenderer->GetRenderWindow()->GetSize()[0] / mainRenderer->GetRenderWindow()->GetSize()[1];

//     double minX = center[0] - halfWidth;
//     double maxX = center[0] + halfWidth;
//     double minY = center[1] - parallelScale;
//     double maxY = center[1] + parallelScale;

//     // ----------------------------------------------------
//     // 3. 动态更新网格 (GridActor)
//     // ... (Block 3 保持不变) ...
//     gridPoints->Reset();
//     gridLines->Reset();

//     double startX = std::floor(minX / spacing) * spacing;
//     double startY = std::floor(minY / spacing) * spacing;

//     // 纵向线 (垂直于 X 轴的线)
//     for (double x = startX; x <= maxX; x += spacing) {
//         vtkIdType p1 = gridPoints->InsertNextPoint(x, minY, 0);
//         vtkIdType p2 = gridPoints->InsertNextPoint(x, maxY, 0);
//         vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
//         line->GetPointIds()->SetId(0, p1);
//         line->GetPointIds()->SetId(1, p2);
//         gridLines->InsertNextCell(line);
//     }

//     // 横向线 (垂直于 Y 轴的线)
//     for (double y = startY; y <= maxY; y += spacing) {
//         vtkIdType p1 = gridPoints->InsertNextPoint(minX, y, 0);
//         vtkIdType p2 = gridPoints->InsertNextPoint(maxX, y, 0);
//         vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
//         line->GetPointIds()->SetId(0, p1);
//         line->GetPointIds()->SetId(1, p2);
//         gridLines->InsertNextCell(line);
//     }
//     gridPoints->Modified();
//     gridLines->Modified();


//     // ----------------------------------------------------
//     // 4. 动态更新标签 (LabelFollowers)
//     // ----------------------------------------------------
//     // 清除旧标签
//     for (vtkSmartPointer<vtkFollower> actor : labelFollowers) {
//         mainRenderer->RemoveActor(actor);
//     }
//     labelFollowers.clear();

//     double labelScaleFactor = 40.0;
//     // Y 标签的固定位置 (略低于 X 轴)
//     double yLabelPos = minY - parallelScale / 20.0*1.1; // 使用固定比例，避免依赖 spacing

//     // 仅为 X 轴 (横向) 标签创建 Follower
//     for (double x = startX; x <= maxX; x += spacing) {
//         vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
//         char labelText[50];
//         // 建议修复 X 轴格式化
//         if (spacing < 1) {
//             snprintf(labelText, sizeof(labelText), "%.1f", x);
//         } else {
//             snprintf(labelText, sizeof(labelText), "%.0f", x); // 避免 int 截断
//         }
//         textSource->SetText(labelText);

//         vtkSmartPointer<vtkPolyDataMapper> textMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//         textMapper->SetInputConnection(textSource->GetOutputPort());

//         vtkSmartPointer<vtkFollower> textActor = vtkSmartPointer<vtkFollower>::New();
//         textActor->SetMapper(textMapper);
//         textActor->SetScale(parallelScale / labelScaleFactor, parallelScale / labelScaleFactor, 1.0);

//         // 放置 X 轴标签
//         textActor->SetPosition(x, yLabelPos, 0);
//         textActor->SetCamera(camera);
//         textActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
//         textActor->SetPickable(false);
//         mainRenderer->AddActor(textActor);
//         labelFollowers.append(textActor);
//     }

//     // 遍历 Y 轴的刻度
//     // 💥 修复 Y 轴标签的 X 坐标位置 💥
//     // 放置在 minX 边界左侧一定距离，确保标签可见
//     double labelMargin = 1.2 * (parallelScale / labelScaleFactor); // 标签大小的 1.2 倍作为边距
//     double xLabelPosLeft = minX - labelMargin;

//     for (double y = startY; y <= maxY; y += spacing) {
//         vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
//         char labelText[50];

//         // 💥 修复 Y 轴标签显示值：取负 💥
//         double displayY = y;

//         // 格式化标签文本
//         if (spacing < 1) {
//             snprintf(labelText, sizeof(labelText), "%.1f", displayY);
//         } else {
//             snprintf(labelText, sizeof(labelText), "%.0f", displayY); // 避免 int 截断
//         }
//         textSource->SetText(labelText);

//         vtkSmartPointer<vtkPolyDataMapper> textMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//         textMapper->SetInputConnection(textSource->GetOutputPort());

//         vtkSmartPointer<vtkFollower> textActor = vtkSmartPointer<vtkFollower>::New();
//         textActor->SetMapper(textMapper);
//         textActor->SetScale(parallelScale / labelScaleFactor, parallelScale / labelScaleFactor, 1.0);

//         // 放置在 xLabelPosLeft 处，沿着 Y 轴移动
//         textActor->SetPosition(xLabelPosLeft, y, 0);

//         textActor->SetCamera(camera);
//         textActor->GetProperty()->SetColor(0.0, 0.0, 0.0); // 黑色

//         textActor->SetPickable(false);
//         mainRenderer->AddActor(textActor);
//         labelFollowers.append(textActor);
//     }
//     // ----------------------------------------------------
//     // 5. 最终渲染
//     // ----------------------------------------------------
//     ui->qvtkWidget->renderWindow()->Render();
// }

// 引入必要的 VTK 头文件（如果未在 .h 中包含）：
// #include "vtkLine.h"
// #include "vtkPolyLine.h" // 用于粗黑轴线

// void MainWindow::AddGridAndLabelsDynamically()
// {
//     if (!mainRenderer) return;
//     vtkCamera* camera = mainRenderer->GetActiveCamera();
//     if (!camera) return;

//     double parallelScale = camera->GetParallelScale();

//     // =====================================================
//     // 统一解析屏幕坐标方向
//     // =====================================================
//     double screenY[3];
//     camera->GetViewUp(screenY);
//     vtkMath::Normalize(screenY);

//     double viewDir[3];
//     camera->GetDirectionOfProjection(viewDir);
//     vtkMath::Normalize(viewDir);

//     double screenX[3];
//     vtkMath::Cross(viewDir, screenY, screenX);
//     vtkMath::Normalize(screenX);

//     // -----------------------------
//     // 1. 主/次刻度间距
//     // -----------------------------
//     double viewHeight = parallelScale * 2.0;
//     double idealSpacing = viewHeight / 8.0;
//     double mainSpacing = findNiceNumberSpacing(idealSpacing);
//     if (mainSpacing < 0.1) mainSpacing = 0.1;
//     currentGridSpacing = mainSpacing;

//     int minorDivisions = 5; // MATLAB 风格次刻度分成 5 份
//     double minorSpacing = mainSpacing / minorDivisions;

//     // -----------------------------
//     // 2. 视口边界（覆盖整个屏幕）
//     // -----------------------------
//     double halfWidth = parallelScale * mainRenderer->GetRenderWindow()->GetSize()[0] /
//                        mainRenderer->GetRenderWindow()->GetSize()[1];
//     double minX = -halfWidth;
//     double maxX = halfWidth;
//     double minY = -parallelScale;
//     double maxY = parallelScale;

//     // -----------------------------
//     // 3. 重置网格和刻度线
//     // -----------------------------
//     gridPoints->Reset();
//     gridLines->Reset();
//     tickPoints->Reset();
//     tickLines->Reset();

//     double startX = std::floor(minX / minorSpacing) * minorSpacing;
//     double startY = std::floor(minY / minorSpacing) * minorSpacing;

//     // 纵向网格线
//     for (double x = startX; x <= maxX + 1e-6; x += minorSpacing) {
//         vtkIdType p1 = gridPoints->InsertNextPoint(x, minY, 0);
//         vtkIdType p2 = gridPoints->InsertNextPoint(x, maxY, 0);
//         vtkIdType pts[2] = {p1, p2};
//         gridLines->InsertNextCell(2, pts);
//     }

//     // 横向网格线
//     for (double y = startY; y <= maxY + 1e-6; y += minorSpacing) {
//         vtkIdType p1 = gridPoints->InsertNextPoint(minX, y, 0);
//         vtkIdType p2 = gridPoints->InsertNextPoint(maxX, y, 0);
//         vtkIdType pts[2] = {p1, p2};
//         gridLines->InsertNextCell(2, pts);
//     }

//     // -----------------------------
//     // 4. 主/次刻度线
//     // -----------------------------
//     double tickLength = parallelScale * 0.02;
//     double minorTickLength = tickLength * 0.5; // 次刻度短一些

//     double effectiveXAxisY = 0.0; // 原点固定
//     double effectiveYAxisX = 0.0;

//     // Y 轴刻度 (水平轴)
//     for (double x = startX; x <= maxX + 1e-6; x += minorSpacing) {
//         vtkIdType p1 = tickPoints->InsertNextPoint(x, effectiveXAxisY, 0);
//         vtkIdType p2;

//         // 判断主/次刻度
//         double mod = std::fmod(std::abs(x), mainSpacing);
//         double len = (mod < 1e-6) ? tickLength : minorTickLength;

//         // 无论 x 正负，刻度向“下”画
//         p2 = tickPoints->InsertNextPoint(x, effectiveXAxisY - len, 0);

//         vtkIdType pts[2] = {p1, p2};
//         tickLines->InsertNextCell(2, pts);
//     }

//     // X 轴刻度 (垂直轴)
//     for (double y = startY; y <= maxY + 1e-6; y += minorSpacing) {
//         vtkIdType p1 = tickPoints->InsertNextPoint(effectiveYAxisX, y, 0);
//         vtkIdType p2;

//         double mod = std::fmod(std::abs(y), mainSpacing);
//         double len = (mod < 1e-6) ? tickLength : minorTickLength;

//         // 无论 y 正负，刻度向“左”画
//         p2 = tickPoints->InsertNextPoint(effectiveYAxisX - len, y, 0);

//         vtkIdType pts[2] = {p1, p2};
//         tickLines->InsertNextCell(2, pts);
//     }

//     tickPoints->Modified();
//     tickLines->Modified();

//     // -----------------------------
//     // 5. 坐标轴线 (固定在原点)
//     // -----------------------------
//     vtkPolyData* yAxisData = vtkPolyData::SafeDownCast(xAxisActor->GetMapper()->GetInput());
//     yAxisData->GetPoints()->SetPoint(0, minX, effectiveXAxisY, 0);
//     yAxisData->GetPoints()->SetPoint(1, maxX, effectiveXAxisY, 0);
//     yAxisData->GetPoints()->Modified();
//     xAxisActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
//     xAxisActor->GetProperty()->SetLineWidth(1.0);

//     vtkPolyData* xAxisData = vtkPolyData::SafeDownCast(yAxisActor->GetMapper()->GetInput());
//     xAxisData->GetPoints()->SetPoint(0, effectiveYAxisX, minY, 0);
//     xAxisData->GetPoints()->SetPoint(1, effectiveYAxisX, maxY, 0);
//     xAxisData->GetPoints()->Modified();
//     yAxisActor->GetProperty()->SetColor(0.4, 0.4, 0.4);
//     yAxisActor->GetProperty()->SetLineWidth(1.0);

//     // -----------------------------
//     // 6. 刻度标签 (只显示主刻度)
//     // -----------------------------
//     for (auto actor : labelFollowers) mainRenderer->RemoveActor(actor);
//     labelFollowers.clear();

//     double labelScaleFactor = 40.0;
//     double yLabelOffset = parallelScale * 0.03;
//     double xLabelOffset = parallelScale * 0.07;

//     // Y 轴主刻度标签
//     for (double x = std::ceil(minX / mainSpacing) * mainSpacing; x <= maxX + 1e-6; x += mainSpacing) {
//         vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
//         char label[32];
//         snprintf(label, sizeof(label), mainSpacing < 1 ? "%.1f" : "%.0f", x);
//         textSource->SetText(label);

//         vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//         mapper->SetInputConnection(textSource->GetOutputPort());

//         vtkSmartPointer<vtkFollower> actor = vtkSmartPointer<vtkFollower>::New();
//         actor->SetMapper(mapper);
//         double labelX = x; // 默认
//         double halfWidth = parallelScale / labelScaleFactor * 0.5; // 可调
//             labelX -= halfWidth;
//         actor->SetScale(parallelScale / labelScaleFactor, parallelScale / labelScaleFactor, 1.0);
//         actor->SetPosition(labelX, effectiveXAxisY - yLabelOffset, 0);
//         actor->SetCamera(camera);
//         actor->GetProperty()->SetColor(0, 0, 0);
//         actor->SetPickable(false);
//         mainRenderer->AddActor(actor);
//         labelFollowers.push_back(actor);
//     }



//     // X 轴主刻度标签
//     for (double y = std::ceil(minY / mainSpacing) * mainSpacing; y <= maxY + 1e-6; y += mainSpacing) {
//         vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
//         char label[32];
//         snprintf(label, sizeof(label), mainSpacing < 1 ? "%.1f" : "%.0f", y);
//         textSource->SetText(label);

//         vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//         mapper->SetInputConnection(textSource->GetOutputPort());


//         vtkSmartPointer<vtkFollower> actor = vtkSmartPointer<vtkFollower>::New();
//         actor->SetMapper(mapper);
//         actor->SetScale(parallelScale / labelScaleFactor, parallelScale / labelScaleFactor, 1.0);

//         double labelY = y;
//             double halfHeight = parallelScale / labelScaleFactor ; // 可调
//             labelY += halfHeight;
//         actor->SetPosition(effectiveYAxisX - xLabelOffset, labelY, 0);
//         actor->SetCamera(camera);
//         actor->GetProperty()->SetColor(0, 0, 0);
//         actor->SetPickable(false);
//         mainRenderer->AddActor(actor);
//         labelFollowers.push_back(actor);
//     }

//     // -----------------------------
//     // 7. 原点十字
//     // -----------------------------
//     if (originPoints && originLines) {
//         originPoints->Reset();
//         originLines->Reset();
//         double crossHalfLength = parallelScale * 0.05;
//         double z = 0.1;

//         vtkIdType h0 = originPoints->InsertNextPoint(-crossHalfLength, 0, z);
//         vtkIdType h1 = originPoints->InsertNextPoint(crossHalfLength, 0, z);
//         vtkIdType hPts[2] = {h0, h1};
//         originLines->InsertNextCell(2, hPts);

//         vtkIdType v0 = originPoints->InsertNextPoint(0, -crossHalfLength, z);
//         vtkIdType v1 = originPoints->InsertNextPoint(0, crossHalfLength, z);
//         vtkIdType vPts[2] = {v0, v1};
//         originLines->InsertNextCell(2, vPts);

//         originPoints->Modified();
//         originLines->Modified();
//         originPolyData->Modified();
//     }

//     // -----------------------------
//     // 8. 渲染
//     // -----------------------------
//     ui->qvtkWidget->renderWindow()->Render();
// }

void MainWindow::AddGridAndLabelsDynamically()
{
    if (!mainRenderer || !ui->qvtkWidget->renderWindow() || !gridPoints) return;
    vtkCamera* camera = mainRenderer->GetActiveCamera();
    if (!camera) return;


    // --- 1. 获取相机与视口参数 ---
    double parallelScale = camera->GetParallelScale();
    double focalPoint[3];
    camera->GetFocalPoint(focalPoint);

    int* winSize = ui->qvtkWidget->renderWindow()->GetSize();
    double aspect = (winSize[1] > 0) ? (double)winSize[0] / winSize[1] : 1.0;


    // 当前屏幕能看到的物理范围
    double viewMinX = focalPoint[0] - parallelScale*2;
    double viewMaxX = focalPoint[0] + parallelScale*2;
    double viewMinY = focalPoint[1] - parallelScale* aspect;
    double viewMaxY = focalPoint[1] + parallelScale* aspect;

    double windowCenterY = camera->GetWindowCenter()[1]; // 0~1，正向上偏移
    double worldYOffset = windowCenterY * parallelScale * 2.0; // 乘视口高度，得到世界坐标偏移
    qDebug()<<worldYOffset;

    // --- 2. 动态步长计算 ---
    double viewHeight = parallelScale * 2.0;
    double idealSpacing = viewHeight / 12.0;
    double mainSpacing = findNiceNumberSpacing(idealSpacing);

    // double viewWidth  = viewHeight * aspect; // 保证 X/Y 比例正确

    // double viewMinX = focalPoint[0] - viewWidth / 2.0;
    // double viewMaxX = focalPoint[0] + viewWidth / 2.0;

    // double viewMinY = focalPoint[1] - viewHeight / 2.0;
    // double viewMaxY = focalPoint[1] + viewHeight / 2.0;
    if (mainSpacing < 0.1) mainSpacing = 0.1;

    // --- 3. 范围逻辑优化 (解决“有位无网”的关键) ---
    // 策略：取【视野范围】和【点云边界】的交集后，向外强制扩展 1 个 mainSpacing
    // 这样无论怎么缩放，边缘都不会出现断层

    double drawMinX = std::floor(viewMinX / mainSpacing - 1.0) * mainSpacing;
    double drawMaxX = std::ceil(viewMaxX / mainSpacing + 1.0) * mainSpacing;
    double drawMinY = std::floor(viewMinY / mainSpacing - 1.0) * mainSpacing;
    double drawMaxY = std::ceil(viewMaxY / mainSpacing + 1.0) * mainSpacing;

        // --- 4. 绘制网格线 (确保循环包含边界) ---
        gridPoints->Reset();
        gridLines->Reset();

        // 纵向线 (固定 X，延伸 Y)
        // 使用 1e-6 的微小偏移是为了防止浮点数误差导致最后一根线画不出来
        for (double x = std::max(drawMinX, 0.0); x <= drawMaxX + 1e-6; x += mainSpacing) {
            vtkIdType p1 = gridPoints->InsertNextPoint(x, drawMinY, -0.01);
            vtkIdType p2 = gridPoints->InsertNextPoint(x, drawMaxY, -0.01);
            vtkIdType pts[2] = {p1, p2};
            gridLines->InsertNextCell(2, pts);
        }

        // 横向线 (固定 Y，延伸 X)
        for (double y = drawMinY; y <= drawMaxY + 1e-6; y += mainSpacing) {
            vtkIdType p1 = gridPoints->InsertNextPoint(0.0, y, -0.01);
            vtkIdType p2 = gridPoints->InsertNextPoint(drawMaxX, y, -0.01);
            vtkIdType pts[2] = {p1, p2};
            gridLines->InsertNextCell(2, pts);
        }


    gridPoints->Modified();
    if (gridActor) {
        gridActor->GetProperty()->SetColor(0.88, 0.88, 0.88);
        gridActor->GetProperty()->SetLineWidth(1.0);
    }

    // --- 5. 坐标轴与标签位置更新 ---
    // 标签偏移量
    double yLabelOffset = parallelScale * 0.05;
    double xLabelOffset = parallelScale * 0.1;


    // 坐标轴位置：固定在原点，若原点不在视野内则吸附到边缘effectiveYAxisX

    double effectiveXAxisY = std::clamp(0.0, viewMinY + yLabelOffset*2, viewMaxY - yLabelOffset);
    double effectiveYAxisX = std::clamp(0.0, viewMinX +viewHeight-xLabelOffset, viewMaxX - xLabelOffset);


    auto snapToGrid = [&](double v) {
        return std::round(v / mainSpacing) * mainSpacing;
    };

    effectiveYAxisX = snapToGrid(effectiveYAxisX);
    effectiveXAxisY = snapToGrid(effectiveXAxisY);

    // double effectiveYAxisX = std::clamp(0.0, (viewMaxX+viewMinX)/2.0 , viewMaxX - xLabelOffset);
    qDebug()<<effectiveYAxisX;
    qDebug()<<viewMinX;
    qDebug()<<viewMaxX;
    // 更新轴线几何（确保调用 PolyData 的 Modified）
    auto updateAxis = [&](vtkActor* actor, double x1, double y1, double x2, double y2) {
        if (!actor) return;
        vtkPolyData* pd = vtkPolyData::SafeDownCast(actor->GetMapper()->GetInput());
        pd->GetPoints()->SetPoint(0, x1, y1, 0.0);
        pd->GetPoints()->SetPoint(1, x2, y2, 0.0);
        pd->GetPoints()->Modified();
        pd->Modified();
        actor->GetProperty()->SetColor(0.2, 0.2, 0.2);
        actor->GetProperty()->SetLineWidth(1.2);
    };


    updateAxis(xAxisActor, 0.0, effectiveXAxisY, drawMaxX, effectiveXAxisY);
    updateAxis(yAxisActor, effectiveYAxisX, drawMinY, effectiveYAxisX, drawMaxY);




    // --- 6. 标签对象池 ---
    double labelScale = parallelScale / 45.0;
    for (auto actor : labelFollowers) actor->VisibilityOff();
    int labelIdx = 0;

    auto showLabel = [&](double px, double py, double val) {
        if (labelIdx >= (int)labelFollowers.size()) {
            vtkNew<vtkVectorText> vt;
            vtkNew<vtkPolyDataMapper> m; m->SetInputConnection(vt->GetOutputPort());
            vtkNew<vtkFollower> f; f->SetMapper(m);
            mainRenderer->AddActor(f);
            labelFollowers.push_back(f);
        }
        vtkFollower* actor = labelFollowers[labelIdx++];
        char buf[16];
        snprintf(buf, sizeof(buf), mainSpacing < 1 ? "%.1f" : "%.0f", val);
        vtkVectorText::SafeDownCast(actor->GetMapper()->GetInputAlgorithm())->SetText(buf);
        actor->SetScale(labelScale, labelScale, 1.0);
        actor->SetCamera(camera);
        actor->SetPosition(px, py, 0.01);
        actor->GetProperty()->SetColor(0.1, 0.1, 0.1);
        actor->VisibilityOn();
    };


    // 绘制标签 (只在视野内)
    for (double x = drawMinX; x <= drawMaxX; x += mainSpacing) {
        if (x <= 0.0 || x > viewMaxX) continue;

        showLabel(x - (labelScale * 0.5), effectiveXAxisY - yLabelOffset*0.1, x);
    }

    for (double y = drawMinY; y <= drawMaxY; y += mainSpacing) {
        if (y < viewMinY || y > viewMaxY) continue;

        showLabel(effectiveYAxisX - xLabelOffset*0.5, y + (labelScale), y);
    }

    ui->qvtkWidget->renderWindow()->Render();
}


void MainWindow::calculateCloudMetrics(std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cloud)
{
    // 1. 安全检查
    if (!cloud || cloud->empty()) {
        // 如果点云为空，可以给一组默认值，防止相机缩放到 0
        cloudCenterX = 0.0;
        cloudCenterY = 0.0;
        suggestedParallelScale = 10.0;

        return;
    }

    // 2. 初始化极值
    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY = minX;
    float maxY = maxX;

    // 3. 遍历计算边界
    for (const auto& p : cloud->points) {
        if (p.x < minX) minX = p.x; if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y; if (p.y > maxY) maxY = p.y;
    }

    // 4. 存储结果到成员变量
    currentCloudBounds[0] = minX;
    currentCloudBounds[1] = maxX;
    currentCloudBounds[2] = minY;
    currentCloudBounds[3] = maxY;

    cloudCenterX = (minX + maxX) / 2.0;
    cloudCenterY = (minY + maxY) / 2.0;

    // 5. 计算缩放比例
    double spanX = maxX - minX;
    double spanY = maxY - minY;
    double maxSpan = (spanX > spanY) ? spanX : spanY;

    // 预留边距逻辑：maxSpan * 0.6 是为了让点云占据屏幕约 80% 的区域
    // 设定一个最小值（如 5.0），防止只有单个点时缩放太夸张
    suggestedParallelScale = (maxSpan < 1.0) ? 5.0 : maxSpan * 0.6;

    // 调试打印 (可选)
    qDebug() << "Metrics Updated - Center:(" << cloudCenterX << "," << cloudCenterY
             << ") Scale:" << suggestedParallelScale;
}

// 辅助函数：找到一个“漂亮”的数字间隔 (例如 1, 2, 5, 10, 20, 50, 100...)
    // 找到一个“漂亮”的数字间隔 (主刻度间距)
    // 找到一个“漂亮”的数字间隔 (主刻度间距)
    // 找到 MATLAB 风格“漂亮数字”主刻度间距
    double MainWindow::findNiceNumberSpacing(double idealSpacing)
    {
        if (idealSpacing <= 0) return 1.0;

        double magnitude = std::pow(10.0, std::floor(std::log10(idealSpacing)));
        double fraction = idealSpacing / magnitude;

        if (fraction < 1.5)
            return 1.0 * magnitude;
        else if (fraction < 3.0)
            return 2.0 * magnitude;
        else if (fraction < 7.0)
            return 5.0 * magnitude;
        else
            return 10.0 * magnitude;
    }



void MainWindow::AddYlabel(double sizeX,double sizeY){

    if (!mainRenderer)
    {
        mainRenderer = vtkSmartPointer<vtkRenderer>::New();
        ui->qvtkWidget->renderWindow()->AddRenderer(mainRenderer);
    }
    double xOffset = 0;
    double yLabelPos = -sizeY / 2 ;  // 下方偏移，避免挡住网格
    double xLabels[] = {25,50,75,100,125,150,175,200,225,250,275,300};

    for (int i = 0; i < 14; ++i) {
        double xPos = -xLabels[i] + xOffset;

        // 使用 vtkVectorText 生成文字
        vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
        char labelText[20];
        snprintf(labelText, sizeof(labelText), "- %dm", (int)xLabels[i]);
        textSource->SetText(labelText);

        // 文字映射
        vtkSmartPointer<vtkPolyDataMapper> textMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        textMapper->SetInputConnection(textSource->GetOutputPort());

        // Follower，让文字始终面向相机
        vtkSmartPointer<vtkFollower> textActor = vtkSmartPointer<vtkFollower>::New();
        textActor->SetMapper(textMapper);
        textActor->SetScale(5, 5, 5); // 调整大小
        textActor->SetPosition(xPos, yLabelPos, 0);
        textActor->SetCamera(mainRenderer->GetActiveCamera());

        textActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
        mainRenderer->AddActor(textActor);
    }
}

void MainWindow::setSectorStartAngle(int value)
{
    // 确保角度在 0-360 度范围内 (可选的边界检查)
    if (value >= 0 && value <= 360) {
        // 🌟 关键：int 转换为 double 🌟
        sectorStartAngle = static_cast<double>(value);

        // 触发几何体更新
        updateSectorGeometry();
    }
}

void MainWindow::setSectorEndAngle(int value)
{
    // 确保角度在 0-360 度范围内 (可选的边界检查)
    if (value >= 0 && value <= 360) {
        // 🌟 关键：int 转换为 double 🌟
        sectorEndAngle = static_cast<double>(value);

        // 触发几何体更新
        updateSectorGeometry();
    }
}

void MainWindow::createSectorGeometry()
{
    // 🌟 修正 1：如果 Actor 已经存在，则直接退出（同时检查背景和前景）
    // 确保 sectorBgActor 也被检查，避免重复添加
    if (sectorActor || sectorBgActor) {
        return;
    }

    // 1. 动态获取几何参数
    double centerX = 0, centerY = 0, centerZ = 0;

    // 🌟 修正 2：半径使用 userXRange 和 userYRange 的最小值 🌟
    // 注意：这里的 radius 仅用于首次创建，后续更新需通过 updateSectorGeometry
    double radius = std::min(userXRange, userYRange);

    // 🌟 修正 3：角度使用成员变量（来自 SpinBox）🌟
    double startAngleDeg = sectorStartAngle;
    double endAngleDeg = sectorEndAngle;
    int resolution = 50;

    // 2. 几何体创建逻辑
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();

    // 中心点
    points->InsertNextPoint(centerX, centerY, centerZ);

    for (int i = 0; i <= resolution; ++i) {
        double angleDeg = startAngleDeg + (endAngleDeg - startAngleDeg) * i / resolution;
        double angleRad = vtkMath::RadiansFromDegrees(angleDeg);
        double x = centerX + radius * sin(angleRad);
        double y = centerY + radius * cos(angleRad);
        double z = centerZ;
        points->InsertNextPoint(x, y, z);
    }

    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(resolution + 2); // 中心点 + 边界点数

    for (vtkIdType i = 0; i < resolution + 2; ++i) {
        polygon->GetPointIds()->SetId(i, i);
    }

    polygons->InsertNextCell(polygon);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    // 3. 🌟 修正 4：创建并添加背景 Actor 🌟
    // sectorBgActor 成员变量必须在 mainwindow.h 中声明
    sectorBgActor = vtkSmartPointer<vtkActor>::New();
    sectorBgActor->SetMapper(mapper);
    sectorBgActor->GetProperty()->SetColor(0.8, 0.8, 0.8); // 浅灰色背景
    sectorBgActor->GetProperty()->SetOpacity(0.1);         // 完全不透明
    sectorBgActor->SetPickable(false);
    mainRenderer->AddActor(sectorBgActor);

    // 4. 创建并添加前景 Actor
    sectorActor = vtkSmartPointer<vtkActor>::New();
    sectorActor->SetMapper(mapper);
    sectorActor->GetProperty()->SetColor(0.8, 0.8, 0.8); // 黄色
    sectorActor->GetProperty()->SetOpacity(0.1);
    sectorActor->SetPickable(false); //
    mainRenderer->AddActor(sectorActor);
}

void MainWindow::updateSectorGeometry()
{
    if (!sectorActor || !sectorBgActor) return; // 确保 Actor 已存在

    // 1. 重新计算几何体
    // 🌟 关键：将 createSectorGeometry 中所有几何体创建代码复制到这里 🌟

    double centerX = 0, centerY = 0, centerZ = 0;
    double radius = std::min(userXRange, userYRange); // 动态半径
    double startAngleDeg = sectorStartAngle;
    double endAngleDeg = sectorEndAngle;
    int resolution = 50;

    // ... (复制 Points, Polygons, Polygon 的创建和插入逻辑) ...
    // 2. 几何体创建逻辑
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();

    // 中心点
    points->InsertNextPoint(centerX, centerY, centerZ);

    for (int i = 0; i <= resolution; ++i) {
        double angleDeg = startAngleDeg + (endAngleDeg - startAngleDeg) * i / resolution;
        double angleRad = vtkMath::RadiansFromDegrees(angleDeg);
        double x = centerX + radius * sin(angleRad);
        double y = centerY + radius * cos(angleRad);
        double z = centerZ;
        points->InsertNextPoint(x, y, z);
    }

    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(resolution + 2); // 中心点 + 边界点数

    for (vtkIdType i = 0; i < resolution + 2; ++i) {
        polygon->GetPointIds()->SetId(i, i);
    }

    polygons->InsertNextCell(polygon);


    // 2. 创建新的 PolyData
    vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
    newPolyData->SetPoints(points);
    newPolyData->SetPolys(polygons);

    // 3. 更新 Mapper
    vtkPolyDataMapper* mapper = vtkPolyDataMapper::SafeDownCast(sectorActor->GetMapper());
    if (mapper) {
        mapper->SetInputData(newPolyData); // 设置新的几何数据
        mapper->Update();
    }

    // 3. 更新 sectorActor 的 Mapper
    vtkPolyDataMapper* actorMapper = vtkPolyDataMapper::SafeDownCast(sectorActor->GetMapper());
    if (actorMapper) {
        actorMapper->SetInputData(newPolyData);
        actorMapper->Update();
    }

    // 🌟 关键修正：更新 sectorBgActor 的 Mapper 🌟
    vtkPolyDataMapper* bgMapper = vtkPolyDataMapper::SafeDownCast(sectorBgActor->GetMapper());
    if (bgMapper) {
        bgMapper->SetInputData(newPolyData);
        bgMapper->Update();
    }

    // 4. 重新渲染场景
    ui->qvtkWidget->renderWindow()->Render();
}
void MainWindow::AddSector(double centerX, double centerY, double centerZ,
                           double radius, double startAngleDeg, double endAngleDeg, int resolution)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> polygons = vtkSmartPointer<vtkCellArray>::New();

    // 中心点
    points->InsertNextPoint(centerX, centerY, centerZ);

    for (int i = 0; i <= resolution; ++i) {
        double angleDeg = startAngleDeg + (endAngleDeg - startAngleDeg) * i / resolution;
        double angleRad = vtkMath::RadiansFromDegrees(angleDeg);
        double x = centerX + radius * cos(angleRad);
        double y = centerY + radius * sin(angleRad);
        double z = centerZ;
        points->InsertNextPoint(x, y, z);
    }

    vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
    polygon->GetPointIds()->SetNumberOfIds(resolution + 2);  // 中心点 + 边界点数

    for (vtkIdType i = 0; i < resolution + 2; ++i) {
        polygon->GetPointIds()->SetId(i, i);
    }

    polygons->InsertNextCell(polygon);

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(1,1,1);  // 橙色 1.0, 1.0, 0.0
    actor->GetProperty()->SetOpacity(0.1);           // 半透明

    mainRenderer->AddActor(actor);
}

vtkSmartPointer<vtkActor> MainWindow::createGrid(double spacing, int count )
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    for (int i = -count; i <= count; ++i)
    {
        double pos = i * spacing;

        // X lines
        vtkIdType p1 = points->InsertNextPoint(-count * spacing, pos, 0);
        vtkIdType p2 = points->InsertNextPoint(count * spacing, pos, 0);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);

        // Y lines
        vtkIdType p3 = points->InsertNextPoint(pos, -count * spacing, 0);
        vtkIdType p4 = points->InsertNextPoint(pos, count * spacing, 0);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p3);
        lines->InsertCellPoint(p4);
    }

    vtkSmartPointer<vtkPolyData> gridData = vtkSmartPointer<vtkPolyData>::New();
    gridData->SetPoints(points);
    gridData->SetLines(lines);

    vtkSmartPointer<vtkPolyDataMapper> gridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    gridMapper->SetInputData(gridData);

    vtkSmartPointer<vtkActor> gridActor = vtkSmartPointer<vtkActor>::New();
    gridActor->SetMapper(gridMapper);
    gridActor->GetProperty()->SetColor(0.5, 0.5, 0.5);
    gridActor->GetProperty()->SetLineWidth(1.0);

    return gridActor;
}


vtkSmartPointer<vtkTextActor> MainWindow::createTextLabel(const std::string& text, double x, double y, double z)
{
    vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
    textActor->SetInput(text.c_str());
    textActor->SetPosition(x, y);  // 2D屏幕位置
    textActor->GetTextProperty()->SetFontSize(12);
    textActor->GetTextProperty()->SetColor(1.0, 1.0, 1.0);  // 白色字体
    return textActor;
}

void MainWindow::setupVTKWidget()
{
    updateLayout();
    setupBackgroud();
    ui->qvtkWidget->renderWindow()->Render();
    //pointPicker = vtkSmartPointer<vtkPointPicker>::New();


    // 初始化区域选择器
    areaPicker = vtkSmartPointer<vtkAreaPicker>::New();
    //ui->qvtkWidget->renderWindow()->GetInteractor()->SetPicker(areaPicker);

    // 右下角子渲染器
    selectedRenderer = vtkSmartPointer<vtkRenderer>::New();
    selectedRenderer->SetViewport(0.75, 0.0, 1.0, 0.25);  // 右下角
    selectedRenderer->SetBackground(0.2, 0.2, 0.2);
    // 右下角子渲染器先不显示
    //ui->qvtkWidget->renderWindow()->AddRenderer(selectedRenderer);



    ui->qvtkWidget->installEventFilter(this);
}
void MainWindow::SatalitePointCloud(){
    satecloud = customdata.SateliteRadarData();

    vtkSmartPointer<vtkPolyData> polyData = PointCloudLoader::convertmydataToVTKPolyData(satecloud);

    vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    glyphFilter->SetInputData(polyData);
    glyphFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyphFilter->GetOutputPort());

    // 显示 Speed 字段作为颜色
    mapper->SetScalarModeToUsePointFieldData();
    mapper->SelectColorArray("Speed");
    mapper->ScalarVisibilityOn();

    // 自动范围（如果不设置 colormap）
    double range[2];
    polyData->GetPointData()->GetArray("Speed")->GetRange(range);
    mapper->SetScalarRange(range);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);


    //double pointSize = std::clamp(1000.0 / static_cast<double>(satecloud->points.size()), 1.0, 10.0);
    actor->GetProperty()->SetPointSize(5.0);

    //vtkRenderer* renderer = ui->qvtkWidget->renderWindow()->GetRenderers()->GetFirstRenderer();
    mainRenderer->RemoveAllViewProps();  // 清空旧内容
    mainRenderer->AddActor(actor);
    //renderer->ResetCamera();


    double cx = 0, cy = 0, cz = 0;
    size_t n = satecloud->points.size();
    if (n > 0) {
        float minX = std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = -std::numeric_limits<float>::max();

        for (const auto& p : satecloud->points) {
            cx += p.x;
            cy += p.y;
            cz += p.z;

            if (p.x < minX) minX = p.x;
            if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.y > maxY) maxY = p.y;
            if (p.z < minZ) minZ = p.z;
            if (p.z > maxZ) maxZ = p.z;
        }
        cx /= n;
        cy /= n;
        cz /= n;

        // 计算最大边长
        float maxLength = std::max({ maxX - minX, maxY - minY, maxZ - minZ });


        // 设置坐标系位置和大小
        dynamicAxesActor->SetPosition(cx, cy, cz);
        dynamicAxesActor->SetTotalLength(maxLength * 0.3, maxLength * 0.3, maxLength * 0.3);

        float spacing = maxLength / 10;
        if (spacing < 1.0) spacing = 1.0; // 防止太小

        gridActor = createGrid(spacing, 10); // 生成动态网格
        mainRenderer->AddActor(gridActor);



        for (int i = -10; i <= 10; ++i)
        {
            double pos = i * spacing;
            std::string label = std::to_string(static_cast<int>(pos));

            // X 轴标注
            vtkSmartPointer<vtkBillboardTextActor3D> xLabel = vtkSmartPointer<vtkBillboardTextActor3D>::New();
            xLabel->SetInput(label.c_str());
            xLabel->SetPosition(pos, -10 * spacing - spacing * 0.5, 0);  // 放在网格下方一点
            xLabel->GetTextProperty()->SetFontSize(12);
            xLabel->GetTextProperty()->SetColor(0.7, 0.7, 0.7);  // 灰色
            mainRenderer->AddActor(xLabel);

            // Y 轴标注
            vtkSmartPointer<vtkBillboardTextActor3D> yLabel = vtkSmartPointer<vtkBillboardTextActor3D>::New();
            yLabel->SetInput(label.c_str());
            yLabel->SetPosition(-10 * spacing - spacing * 0.5, pos, 0);  // 放在网格左边一点
            yLabel->GetTextProperty()->SetFontSize(12);
            yLabel->GetTextProperty()->SetColor(0.7, 0.7, 0.7);
            mainRenderer->AddActor(yLabel);
        }
    }

    mainRenderer->AddActor(dynamicAxesActor); // 确保坐标轴不被清除

    if (isFirstFrame) {
        mainRenderer->ResetCamera();
        isFirstFrame = false;
    }




    ui->qvtkWidget->renderWindow()->Render();

    //displayPointCloud();

}
void MainWindow::updatePointCloud()
{
    //RadarSimulator simulator;
    //cloud = simulator.generateScanFrame();
   // cloud = simulator.generate16LineScanFrame();








    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>  copyCloud(new pcl::PointCloud<PointXYZRGBWithProperties>);


    //boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> copyCloud(new pcl::PointCloud<pcl::PointXYZRGB>);
    *copyCloud = *satecloud;

    // 保持multiFrames最多10帧
    /*if (multiFrames.size() >= 10) {
        multiFrames.erase(multiFrames.begin()); // 删除最旧一帧
    }*/
    multiFrames.push_back(copyCloud);

    displayPointCloud();
}
// void MainWindow::displayPointCloud()
// {

//     if (ui->playframecount) {
//         // 读取 QSpinBox 的值，并更新成员变量
//         maxFramesToAccumulate = ui->playframecount->value();

//         // 确保积累帧数至少为 1
//         if (maxFramesToAccumulate < 1) {
//             maxFramesToAccumulate = 1;
//         }
//     } else {
//         // 如果 UI 控件未初始化或不存在，使用硬编码的默认值
//         maxFramesToAccumulate = 5;
//     }

//     if (!satecloud || satecloud->empty()) {
//         if (pointCloudActor) {
//             mainRenderer->RemoveActor(pointCloudActor);
//             pointCloudActor = nullptr; // 可选：如果您的设计允许
//         }
//         mainRenderer->Render();
//         return;    }

//     std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> filteredCloud;

//     filteredCloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();

//     if (currentFilter.enabled && satecloud && !satecloud->empty()) {
//         for (size_t i = 0; i < satecloud->size(); ++i) {
//             // 逐个检查过滤条件
//             auto p = satecloud->points[i];

//             //bool passedFilter = true; // 新增一个标记，避免重复 continue
//             if (p.range < currentFilter.rangeMin || p.range > currentFilter.rangeMax) continue;
//             if (p.dopplerSpeed < currentFilter.dopplerMin || p.dopplerSpeed > currentFilter.dopplerMax) continue;
//             if (p.powerdB < currentFilter.powerdBMin || p.powerdB > currentFilter.powerdBMax) continue;
//             if (p.SNRdB < currentFilter.snrMin || p.SNRdB > currentFilter.snrMax) continue;
//             if (p.Q_azi < currentFilter.qAziMin || p.Q_azi > currentFilter.qAziMax) continue;
//             if (p.Q_ele < currentFilter.qEleMin || p.Q_ele > currentFilter.qEleMax) continue;
//             if (p.azimuthAng < currentFilter.aziAngMin || p.azimuthAng > currentFilter.aziAngMax) continue;
//             if (p.eleAng < currentFilter.eleAngMin || p.eleAng > currentFilter.eleAngMax) continue;
//             if (p.radVelAbs < currentFilter.radVelAbsMin || p.radVelAbs > currentFilter.radVelAbsMax) continue;
//             if (p.rcsdB < currentFilter.rcsdBMin || p.rcsdB > currentFilter.rcsdBMax) continue;

//             // detValid 筛选
//             const float eps = 1e-6f;
//             if (currentFilter.detValidMode == FilterCriteria::Only0 && std::abs(p.detValid - 0.0f) > eps) continue;
//             if (currentFilter.detValidMode == FilterCriteria::Only1 && std::abs(p.detValid - 1.0f) > eps) continue;


//                 // 🌟 关键修正：保存原始索引 🌟
//                 p.originalIndex = (int)i;
//                 filteredCloud->push_back(p);

//            // filteredCloud->push_back(p);
//         }
//     } else {
//         filteredCloud->points.resize(satecloud->points.size());
//         filteredCloud->width = satecloud->width;
//         filteredCloud->height = satecloud->height;

//         for (size_t i = 0; i < satecloud->size(); ++i) {
//             auto p = satecloud->points[i]; // 拷贝原始点
//             p.originalIndex = (int)i;      // 设置索引
//             filteredCloud->points[i] = p; // 存入 filteredCloud
//         }

//     }


//     //QMessageBox::information(this, "提示",  QString("筛选后点云数量: %1").arg(filteredCloud->size()));

//     // PCL点云转VTK PolyData
//     //vtkSmartPointer<vtkPolyData> polyData = PointCloudLoader::convertmydataToVTKPolyData(filteredCloud);

//     // 3. 帧积累管理 (新逻辑)
//     // ------------------------------------------------------------------
//     bool isNewFrame = (satecloud != lastAccumulatedCloud);

//     bool shouldUpdateFrameBuffer = !colorUpdateOnly && (isNewFrame || filterAppliedFlag);

//     if (shouldUpdateFrameBuffer){

//         if (filterAppliedFlag) {
//             frameBuffer.clear();
//             filterAppliedFlag = false; // 清除标志，只清空一次
//         }
//     // 3a. 将新帧添加到缓冲区
//     frameBuffer.append(filteredCloud);

//     // 💥 调试点 A：检查新帧加入后缓冲区的状态
//     // qDebug() << "--- Frame #" << frameBuffer.size() << " Added ---";
//     // qDebug() << "maxFramesToAccumulate:" << maxFramesToAccumulate;

//     // 3b. 控制最大帧数
//     while (frameBuffer.size() > maxFramesToAccumulate) {
//         frameBuffer.removeFirst();
//     }

//     // 💥 调试点 B：检查滑动窗口调整后缓冲区的状态
//     // qDebug() << "Buffer size after cleanup:" << frameBuffer.size();

//     // 💥 关键修正：更新追踪器 💥
//     if (isNewFrame) {
//         lastAccumulatedCloud = satecloud; // 标记当前这一帧已经被成功积累
//     }
//     }
//     // 3c. 合并所有帧到一个新的 PCL 点云中
//     std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cumulativeCloud =
//         std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();

//     for (const auto& frame : frameBuffer) {
//         // 使用 PCL 的合并函数或手动合并
//         *cumulativeCloud += *frame;
//     }

//     colorizer.colorizePointCloud(cumulativeCloud, currentColorMode);

//     // 💥 调试点 C：检查最终累积的点云数量
//     // qDebug() << "Total accumulated points:" << cumulativeCloud->size();

//     vtkSmartPointer<vtkPolyData> newPolyData = PointCloudLoader::convertmydataToVTKPolyData(cumulativeCloud);






//     // vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
//     // glyphFilter->SetInputData(polyData);
//     // glyphFilter->Update();




//     // vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//     // mapper->SetInputConnection(glyphFilter->GetOutputPort());



//     // if (!pointCloudActor) {
//     //     pointCloudActor = vtkSmartPointer<vtkActor>::New();
//     //     pointCloudActor->GetProperty()->SetPointSize(5.0);
//     //     mainRenderer->AddActor(pointCloudActor);
//     // }


//     // pointCloudActor->SetPickable(true);
//     // pointCloudActor->GetProperty()->SetPointSize(5.0);
//     // pointCloudActor->SetMapper(mapper);




//     //vtkRenderer* renderer = ui->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer();


//     //mainRenderer->RemoveAllViewProps();
//     //mainRenderer->AddActor(actor);
// /*
//     // 计算点云中心
//     double cx = 0, cy = 0, cz = 0;
//     size_t n = satecloud->points.size();
//     if (n > 0) {
//         float minX = std::numeric_limits<float>::max();
//         float maxX = -std::numeric_limits<float>::max();
//         float minY = std::numeric_limits<float>::max();
//         float maxY = -std::numeric_limits<float>::max();
//         float minZ = std::numeric_limits<float>::max();
//         float maxZ = -std::numeric_limits<float>::max();

//         for (const auto& p : satecloud->points) {
//             cx += p.x;
//             cy += p.y;
//             cz += p.z;

//             if (p.x < minX) minX = p.x;
//             if (p.x > maxX) maxX = p.x;
//             if (p.y < minY) minY = p.y;
//             if (p.y > maxY) maxY = p.y;
//             if (p.z < minZ) minZ = p.z;
//             if (p.z > maxZ) maxZ = p.z;
//         }
//         cx /= n;
//         cy /= n;
//         cz /= n;

//         // 计算最大边长
//         float maxLength = std::max({ maxX - minX, maxY - minY, maxZ - minZ });

//         // 设置坐标系位置和大小
//         dynamicAxesActor->SetPosition(cx, cy, cz);
//         dynamicAxesActor->SetTotalLength(maxLength * 0.3, maxLength * 0.3, maxLength * 0.3);

//         float spacing = maxLength / 10;
//         if (spacing < 1.0) spacing = 1.0; // 防止太小

//         gridActor = createGrid(spacing, 10); // 生成动态网格
//         mainRenderer->AddActor(gridActor);

//         for (int i = -10; i <= 10; ++i)
//         {
//             double pos = i * spacing;
//             std::string label = std::to_string(static_cast<int>(pos));

//             // X 轴标注
//             vtkSmartPointer<vtkBillboardTextActor3D> xLabel = vtkSmartPointer<vtkBillboardTextActor3D>::New();
//             xLabel->SetInput(label.c_str());
//             xLabel->SetPosition(pos, -10 * spacing - spacing * 0.5, 0);  // 放在网格下方一点
//             xLabel->GetTextProperty()->SetFontSize(12);
//             xLabel->GetTextProperty()->SetColor(0.7, 0.7, 0.7);  // 灰色
//             mainRenderer->AddActor(xLabel);

//             // Y 轴标注
//             vtkSmartPointer<vtkBillboardTextActor3D> yLabel = vtkSmartPointer<vtkBillboardTextActor3D>::New();
//             yLabel->SetInput(label.c_str());
//             yLabel->SetPosition(-10 * spacing - spacing * 0.5, pos, 0);  // 放在网格左边一点
//             yLabel->GetTextProperty()->SetFontSize(12);
//             yLabel->GetTextProperty()->SetColor(0.7, 0.7, 0.7);
//             mainRenderer->AddActor(yLabel);
//         }
//     }

//     mainRenderer->AddActor(dynamicAxesActor);  // 确保坐标轴不被清除
// */
//     /* // 计算点云中心
//     double cx = 0, cy = 0, cz = 0;
//     size_t n = cloud->points.size();
//     if (n > 0) {
//         for (const auto& p : cloud->points) {
//             cx += p.x;
//             cy += p.y;
//             cz += p.z;
//         }
//         cx /= n;
//         cy /= n;
//         cz /= n;

//         dynamicAxesActor->SetPosition(cx, cy, cz);
//     }

// */
//     if (isFirstFrame) {

//         // vtkCamera* camera = mainRenderer->GetActiveCamera();
//         //     if (camera) {
//         //         // 🌟 核心：将相机的焦点对准结算出的点云中心
//         //         camera->SetFocalPoint(cloudCenterX, cloudCenterY, 0);

//         //         // 🌟 核心：将相机的位置放在中心点的上方 (Z轴 600)
//         //         camera->SetPosition(cloudCenterX, cloudCenterY, 600);

//         //         // 🌟 核心：应用建议的缩放比例
//         //         if (camera->GetParallelProjection()) {
//         //             camera->SetParallelScale(suggestedParallelScale);
//         //         }

//         //         // 🌟 修正：重置窗口偏移（回归正中心）
//         //         camera->SetWindowCenter(0, 0.8);

//         //         // 2. 关键：因为相机位置变了，必须重新生成网格和标签
//         //         AddGridAndLabelsDynamically();
//         //     }

//             // 1. 初始化持久化数据对象
//             pointCloudPolyData = vtkSmartPointer<vtkPolyData>::New();
//             pointCloudPolyData->ShallowCopy(newPolyData);

//             // 2. 建立管线连接 (只需连接一次)
//             glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
//             glyphFilter->SetInputData(pointCloudPolyData);

//             pointCloudMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//             pointCloudMapper->SetInputConnection(glyphFilter->GetOutputPort());

//             // 配置颜色模式
//             pointCloudMapper->ScalarVisibilityOn();
//             pointCloudMapper->SetScalarModeToUsePointData();
//             if (currentColorMode == ColorMode::Fixed) {
//                     pointCloudMapper->SetColorModeToDirectScalars();
//                 } else {
//                     pointCloudMapper->SetColorModeToMapScalars();
//                 }

//             // 3. 初始化 Actor 并设置静态属性
//             pointCloudActor = vtkSmartPointer<vtkActor>::New();
//             pointCloudActor->SetMapper(pointCloudMapper);
//             pointCloudActor->SetPickable(true);

//             auto prop = pointCloudActor->GetProperty();

//             prop->SetPointSize(6.0);

//             prop->SetLighting(false); // 保持原始色彩，不产生阴影

//             mainRenderer->AddActor(pointCloudActor);

//             // 4. 初始化拾取器 (只需将 Actor 指针加入列表一次)
//             if (pointPicker) {
//                 pointPicker->InitializePickList();
//                 pointPicker->AddPickList(pointCloudActor);
//                 pointPicker->PickFromListOn();
//             }

//             isFirstFrame = false;
//         }
//         else {
//             // --- 🌟 高效更新路径 ---

//             // a. 更新数据源：ShallowCopy 比 DeepCopy 快得多
//             pointCloudPolyData->ShallowCopy(newPolyData);

//             // b. 颜色刷新逻辑优化
//                 // 关键：如果不是 Fixed 模式，我们需要确保 ActiveScalars 指向正确的数值数组
//                 if (currentColorMode == ColorMode::RadialVelocity) {
//                     pointCloudPolyData->GetPointData()->SetActiveScalars("radVelAbs");
//                 } else if (currentColorMode == ColorMode::RCS) {
//                     pointCloudPolyData->GetPointData()->SetActiveScalars("RcsdB");
//                 } else if (currentColorMode == ColorMode::Height) {
//                     pointCloudPolyData->GetPointData()->SetActiveScalars("height");
//                 } else {
//                     // Fixed 模式，恢复到默认颜色数组 (通常名为 "RGB" 或 "colors")
//                     if (auto scalars = pointCloudPolyData->GetPointData()->GetScalars()) {
//                         pointCloudPolyData->GetPointData()->SetActiveScalars(scalars->GetName());
//                     }
//                 }

//                 // 显式通知数据已更改
//                 if (auto activeScalars = pointCloudPolyData->GetPointData()->GetScalars()) {
//                     activeScalars->Modified();
//                 }
//                 pointCloudPolyData->Modified();
//         }

//         // --- 5. 渲染全局配置 ---
//         if (pointPicker) {
//             pointPicker->SetTolerance(0.005);
//             // pointPicker->Modified(); // 只有更改了 PickList 才需要调用
//         }

//         mainRenderer->ResetCameraClippingRange();

//         if (currentColorMode == ColorMode::RadialVelocity) {
//             pointCloudMapper->SetColorModeToMapScalars();
//             pointCloudMapper->SelectColorArray("radVelAbs");
//             auto lut = createRadarLUT();
//             updateColorBar("radVelAbs", lut); // 使用你 convert 函数里存的名字
//         } else if (currentColorMode == ColorMode::RCS) {
//             auto lut = createRadarLUT(); // 或者换一个色系
//             updateColorBar("RcsdB", lut);

//         }else if (currentColorMode == ColorMode::Height) {
//             // 方案 A：如果你在 convert 里存了名为 "Height" 的数组（即 pt.z）
//             auto lut = createHeightLUT(); // 或者换一个色系
//             updateColorBar("height", lut);
//         }

//         else {
//             // 如果是 Fixed 模式，隐藏 Color Bar，回到手动颜色
//             if(scalarBarActor) scalarBarActor->SetVisibility(false);
//             pointCloudMapper->SetColorModeToDirectScalars();
//         }


//         ui->qvtkWidget->renderWindow()->Render();
// }

//测试

void MainWindow::displayPointCloud()
{

    if (ui->playframecount) {
        // 读取 QSpinBox 的值，并更新成员变量
        maxFramesToAccumulate = ui->playframecount->value();

        // 确保积累帧数至少为 1
        if (maxFramesToAccumulate < 1) {
            maxFramesToAccumulate = 1;
        }
    } else {
        // 如果 UI 控件未初始化或不存在，使用硬编码的默认值
        maxFramesToAccumulate = 5;
    }

    if (!satecloud || satecloud->empty()) {
        if (pointCloudActor) {
            mainRenderer->RemoveActor(pointCloudActor);
            pointCloudActor = nullptr; // 可选：如果您的设计允许
        }
        mainRenderer->Render();
        return;    }

    if (!filteredCloudPtr) {
        filteredCloudPtr = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();
    }

    filteredCloudPtr->clear();

    if (currentFilter.enabled && satecloud && !satecloud->empty()) {
        filteredCloudPtr->reserve(satecloud->size());
        for (size_t i = 0; i < satecloud->size(); ++i) {
            // 逐个检查过滤条件
            auto p = satecloud->points[i];

            //bool passedFilter = true; // 新增一个标记，避免重复 continue
            if (p.range < currentFilter.rangeMin || p.range > currentFilter.rangeMax) continue;
            if (p.dopplerSpeed < currentFilter.dopplerMin || p.dopplerSpeed > currentFilter.dopplerMax) continue;
            if (p.powerdB < currentFilter.powerdBMin || p.powerdB > currentFilter.powerdBMax) continue;
            if (p.SNRdB < currentFilter.snrMin || p.SNRdB > currentFilter.snrMax) continue;
            if (p.Q_azi < currentFilter.qAziMin || p.Q_azi > currentFilter.qAziMax) continue;
            if (p.Q_ele < currentFilter.qEleMin || p.Q_ele > currentFilter.qEleMax) continue;
            if (p.azimuthAng < currentFilter.aziAngMin || p.azimuthAng > currentFilter.aziAngMax) continue;
            if (p.eleAng < currentFilter.eleAngMin || p.eleAng > currentFilter.eleAngMax) continue;
            if (p.radVelAbs < currentFilter.radVelAbsMin || p.radVelAbs > currentFilter.radVelAbsMax) continue;
            if (p.rcsdB < currentFilter.rcsdBMin || p.rcsdB > currentFilter.rcsdBMax) continue;

            // detValid 筛选
            const float eps = 1e-6f;
            if (currentFilter.detValidMode == FilterCriteria::Only0 && std::abs(p.detValid - 0.0f) > eps) continue;
            if (currentFilter.detValidMode == FilterCriteria::Only1 && std::abs(p.detValid - 1.0f) > eps) continue;


                // 🌟 关键修正：保存原始索引 🌟
                p.originalIndex = (int)i;
                filteredCloudPtr->push_back(p);

           // filteredCloud->push_back(p);
        }
    } else {
        *filteredCloudPtr = *satecloud;

                // 快速设置索引
                for (size_t i = 0; i < filteredCloudPtr->size(); ++i) {
                    filteredCloudPtr->points[i].originalIndex = static_cast<int>(i);
                }

    }



    // 3. 帧积累管理 (新逻辑)
    // ------------------------------------------------------------------
    bool isNewFrame = (satecloud != lastAccumulatedCloud);

    bool shouldUpdateFrameBuffer = !colorUpdateOnly && (isNewFrame || filterAppliedFlag);

    if (shouldUpdateFrameBuffer){

        if (filterAppliedFlag) {
            frameBuffer.clear();
            filterAppliedFlag = false; // 清除标志，只清空一次
        }
    // 3a. 将新帧添加到缓冲区
    frameBuffer.append(std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>(*filteredCloudPtr));

    // 💥 调试点 A：检查新帧加入后缓冲区的状态
    // qDebug() << "--- Frame #" << frameBuffer.size() << " Added ---";
    // qDebug() << "maxFramesToAccumulate:" << maxFramesToAccumulate;

    // 3b. 控制最大帧数
    while (frameBuffer.size() > maxFramesToAccumulate) {
        frameBuffer.removeFirst();
    }

    // 💥 调试点 B：检查滑动窗口调整后缓冲区的状态
    // qDebug() << "Buffer size after cleanup:" << frameBuffer.size();

    // 💥 关键修正：更新追踪器 💥
    if (isNewFrame) {
        lastAccumulatedCloud = satecloud; // 标记当前这一帧已经被成功积累
    }
    }
    // 3c. 合并所有帧到一个新的 PCL 点云中
    if (!cumulativeCloud) {
        cumulativeCloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();
    }

    // 1. 性能优化：先计算所有帧的总点数，避免合并过程中反复触发内存分配
    size_t totalPoints = 0;
    for (const auto& frame : frameBuffer) {
        totalPoints += frame->size();
    }

    cumulativeCloud->points.resize(totalPoints);
    cumulativeCloud->width = static_cast<uint32_t>(totalPoints);
    cumulativeCloud->height = 1;

    // 3. 性能优化：使用顺序拷贝（std::copy）替代 operator+=
    // 这样可以确保数据是连续线性写入内存的，对 CPU 缓存最友好
    size_t currentOffset = 0;
    for (const auto& frame : frameBuffer) {
        if (frame->empty()) continue;

        size_t frameSize = frame->size();
        std::copy(frame->points.begin(),
                  frame->points.end(),
                  cumulativeCloud->points.begin() + currentOffset);

        currentOffset += frameSize;
    }

    colorizer.colorizePointCloud(cumulativeCloud, currentColorMode);

    // 💥 调试点 C：检查最终累积的点云数量
    // qDebug() << "Total accumulated points:" << cumulativeCloud->size();

    vtkSmartPointer<vtkPolyData> newPolyData = PointCloudLoader::convertmydataToVTKPolyData(cumulativeCloud);



    if (isFirstFrame) {



            // 1. 初始化持久化数据对象
            pointCloudPolyData = vtkSmartPointer<vtkPolyData>::New();
            pointCloudPolyData->ShallowCopy(newPolyData);

            // 2. 建立管线连接 (只需连接一次)
            glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
            glyphFilter->SetInputData(pointCloudPolyData);

            pointCloudMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            pointCloudMapper->SetInputConnection(glyphFilter->GetOutputPort());

            // 配置颜色模式
            pointCloudMapper->ScalarVisibilityOn();
            pointCloudMapper->SetScalarModeToUsePointData();
            if (currentColorMode == ColorMode::Fixed) {
                    pointCloudMapper->SetColorModeToDirectScalars();
                } else {
                    pointCloudMapper->SetColorModeToMapScalars();
                }

            // 3. 初始化 Actor 并设置静态属性
            pointCloudActor = vtkSmartPointer<vtkActor>::New();
            pointCloudActor->SetMapper(pointCloudMapper);
            pointCloudActor->SetPickable(true);

            auto prop = pointCloudActor->GetProperty();

            prop->SetPointSize(6.0);

            prop->SetLighting(false); // 保持原始色彩，不产生阴影

            mainRenderer->AddActor(pointCloudActor);

            // 4. 初始化拾取器 (只需将 Actor 指针加入列表一次)
            if (pointPicker) {
                pointPicker->InitializePickList();
                pointPicker->AddPickList(pointCloudActor);
                pointPicker->PickFromListOn();
            }

            isFirstFrame = false;
        }
        else {
            // --- 🌟 高效更新路径 ---

            // a. 更新数据源：ShallowCopy 比 DeepCopy 快得多
            pointCloudPolyData->ShallowCopy(newPolyData);

            // b. 颜色刷新逻辑优化
                // 关键：如果不是 Fixed 模式，我们需要确保 ActiveScalars 指向正确的数值数组
                if (currentColorMode == ColorMode::RadialVelocity) {
                    pointCloudPolyData->GetPointData()->SetActiveScalars("radVelAbs");
                } else if (currentColorMode == ColorMode::RCS) {
                    pointCloudPolyData->GetPointData()->SetActiveScalars("RcsdB");
                } else if (currentColorMode == ColorMode::Height) {
                    pointCloudPolyData->GetPointData()->SetActiveScalars("height");
                } else if (currentColorMode == ColorMode::SNR) {
                    pointCloudPolyData->GetPointData()->SetActiveScalars("SNRdB");
                }
                else {
                    // Fixed 模式，恢复到默认颜色数组 (通常名为 "RGB" 或 "colors")
                    if (auto scalars = pointCloudPolyData->GetPointData()->GetScalars()) {
                        pointCloudPolyData->GetPointData()->SetActiveScalars(scalars->GetName());
                    }
                }

                // 显式通知数据已更改
                if (auto activeScalars = pointCloudPolyData->GetPointData()->GetScalars()) {
                    activeScalars->Modified();
                }
                pointCloudPolyData->Modified();
        }

        // --- 5. 渲染全局配置 ---
        if (pointPicker) {
            pointPicker->SetTolerance(0.005);
            // pointPicker->Modified(); // 只有更改了 PickList 才需要调用
        }

        mainRenderer->ResetCameraClippingRange();

        if (currentColorMode == ColorMode::RadialVelocity) {
            pointCloudMapper->SetColorModeToMapScalars();
            pointCloudMapper->SelectColorArray("radVelAbs");
            auto lut = createRadarLUT();
            updateColorBar("radVelAbs", lut); // 使用你 convert 函数里存的名字
        } else if (currentColorMode == ColorMode::RCS) {
            auto lut = createRadarLUT(); // 或者换一个色系
            updateColorBar("RcsdB", lut);

        }else if (currentColorMode == ColorMode::Height) {
            // 方案 A：如果你在 convert 里存了名为 "Height" 的数组（即 pt.z）
            auto lut = createHeightLUT(); // 或者换一个色系
            updateColorBar("height", lut);
        }
        else if (currentColorMode == ColorMode::SNR) {
                    // 方案 A：如果你在 convert 里存了名为 "Height" 的数组（即 pt.z）
                    auto lut = createRadarSNRLUT(); // 或者换一个色系
                    updateColorBar("SNRdB", lut);
                }

        else {
            // 如果是 Fixed 模式，隐藏 Color Bar，回到手动颜色
            if(scalarBarActor) scalarBarActor->SetVisibility(false);
            pointCloudMapper->SetColorModeToDirectScalars();
        }


        ui->qvtkWidget->renderWindow()->Render();
}


// void MainWindow::displayPointCloud()
// {
//     // 1. 获取 UI 配置
//     if (ui->playframecount) {
//         maxFramesToAccumulate = qMax(1, ui->playframecount->value());
//     }

//     if (!satecloud || satecloud->empty()) {
//         if (pointCloudActor) pointCloudActor->SetVisibility(false);
//         ui->qvtkWidget->renderWindow()->Render();
//         return;
//     }

//     // 2. 过滤逻辑：使用持久化的 filteredCloudPtr
//     filteredCloudPtr->clear();
//     filteredCloudPtr->reserve(satecloud->size());

//     if (currentFilter.enabled) {
//         for (const auto& p : satecloud->points) {
//             // 范围筛选逻辑...
//             if (p.range < currentFilter.rangeMin || p.range > currentFilter.rangeMax) continue;
//             if (p.dopplerSpeed < currentFilter.dopplerMin || p.dopplerSpeed > currentFilter.dopplerMax) continue;
//             // ... (此处省略你的其他 10 项 continue 检查)

//             filteredCloudPtr->push_back(p);
//         }
//     } else {
//         *filteredCloudPtr = *satecloud;
//     }

//     // 3. 帧积累管理
//     bool isNewFrame = (satecloud != lastAccumulatedCloud);
//     if (!colorUpdateOnly && (isNewFrame || filterAppliedFlag)) {
//         if (filterAppliedFlag) {
//             frameBuffer.clear();
//             filterAppliedFlag = false;
//         }
//         // 深拷贝 filteredCloudPtr 的当前快照存入 buffer
//         auto frameSnapshot = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>(*filteredCloudPtr);
//         frameBuffer.append(frameSnapshot);

//         while (frameBuffer.size() > maxFramesToAccumulate) frameBuffer.removeFirst();
//         if (isNewFrame) lastAccumulatedCloud = satecloud;
//     }

//     // 4. 合并点云 (用于显示)
//     auto cumulativeCloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();
//     for (const auto& frame : frameBuffer) *cumulativeCloud += *frame;

//     // 应用颜色模式 (修改 pt.r, pt.g, pt.b)
//     colorizer.colorizePointCloud(cumulativeCloud, currentColorMode);

//     size_t numPoints = cumulativeCloud->size();
//     if (numPoints == 0) return;

//     // 5. VTK 管线初始化与内存直写
//     if (isFirstFrame || !pointCloudPolyData) {
//         pointCloudPolyData = vtkSmartPointer<vtkPolyData>::New();

//         auto vtkpts = vtkSmartPointer<vtkPoints>::New();
//         vtkpts->SetDataTypeToFloat();
//         pointCloudPolyData->SetPoints(vtkpts);

//         auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
//         colors->SetName("Colors");
//         colors->SetNumberOfComponents(3);
//         pointCloudPolyData->GetPointData()->SetScalars(colors);

//         // 预定义属性数组（例如速度和RCS）
//         auto velArray = vtkSmartPointer<vtkFloatArray>::New();
//         velArray->SetName("radVelAbs");
//         pointCloudPolyData->GetPointData()->AddArray(velArray);

//         if (!scalarBarActor) {
//                 scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
//             }
//             scalarBarActor->SetLookupTable(radarLUT); // 必须绑定构造函数里初始化的那个 radarLUT
//             scalarBarActor->SetTitle("Velocity");
//             scalarBarActor->SetNumberOfLabels(5);

//             // --- 🌟 关键修复：锁定 ColorBar 字体和尺寸 🌟 ---
//                 scalarBarActor->UnconstrainedFontSizeOn(); // 1. 禁用自动字体缩放

//                 // 设置标题样式
//                 auto titleProp = scalarBarActor->GetTitleTextProperty();
//                 titleProp->SetFontSize(14); // 锁定标题为 14 号字
//                 titleProp->BoldOn();
//                 titleProp->SetColor(0.2, 0.2, 0.2); // 别用纯黑，深灰更自然

//                 // 设置标签（数字）样式
//                 auto labelProp = scalarBarActor->GetLabelTextProperty();
//                 labelProp->SetFontSize(12); // 锁定刻度数字为 12 号字
//                 labelProp->BoldOff();
//                 labelProp->SetColor(0.3, 0.3, 0.3);

//                 // 设置在屏幕上的绝对比例（归一化坐标）
//                 scalarBarActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
//                 scalarBarActor->GetPositionCoordinate()->SetValue(0.95, 0.1); // 右下角起始点
//                 scalarBarActor->SetWidth(0.05);  // 宽度占窗口 5%
//                 scalarBarActor->SetHeight(0.45); // 高度占窗口 45% (以前可能默认是 0.8 或更多)

//             // 如果之前没加过，一定要加到 Renderer
//             if (mainRenderer && !mainRenderer->HasViewProp(scalarBarActor)) {
//                 mainRenderer->AddActor2D(scalarBarActor);
//             }


//         glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
//         glyphFilter->SetInputData(pointCloudPolyData);

//         pointCloudMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//         pointCloudMapper->SetInputConnection(glyphFilter->GetOutputPort());

//         pointCloudActor = vtkSmartPointer<vtkActor>::New();
//         pointCloudActor->SetMapper(pointCloudMapper);
//         pointCloudActor->GetProperty()->SetPointSize(6.0);
//         pointCloudActor->GetProperty()->SetLighting(false);

//         mainRenderer->AddActor(pointCloudActor);
//         isFirstFrame = false;
//     }

//     // --- 极速指针拷贝 (Memcpy 级别) ---
//     vtkFloatArray* pData = vtkFloatArray::SafeDownCast(pointCloudPolyData->GetPoints()->GetData());
//     vtkUnsignedCharArray* cData = vtkUnsignedCharArray::SafeDownCast(pointCloudPolyData->GetPointData()->GetScalars());
//     vtkFloatArray* vData = vtkFloatArray::SafeDownCast(pointCloudPolyData->GetPointData()->GetArray("radVelAbs"));

//     if (pData && cData && vData) {
//         // 调整数组大小：如果点数增加会扩容，点数减少则复用内存
//         pData->SetNumberOfTuples(numPoints);
//         cData->SetNumberOfTuples(numPoints);
//         vData->SetNumberOfTuples(numPoints);

//         // 🌟 必须同步更新 Points 的点数计数器
//         pointCloudPolyData->GetPoints()->SetNumberOfPoints(numPoints);

//         float* rawPos = pData->GetPointer(0);
//         unsigned char* rawCol = cData->GetPointer(0);
//         float* rawVel = vData->GetPointer(0);

//         // 极速拷贝
//         for (size_t i = 0; i < numPoints; ++i) {
//             const auto& pt = cumulativeCloud->points[i];

//             // 坐标
//             const size_t posIdx = i * 3;
//             rawPos[posIdx]   = pt.x;
//             rawPos[posIdx+1] = pt.y;
//             rawPos[posIdx+2] = pt.z;

//             // 颜色
//             rawCol[posIdx]   = pt.r;
//             rawCol[posIdx+1] = pt.g;
//             rawCol[posIdx+2] = pt.b;

//             // 映射属性
//             rawVel[i] = pt.radVelAbs;
//         }

//         // 标记数据已更新，触发 GPU 上传
//         pData->Modified();
//         cData->Modified();
//         vData->Modified();
//         pointCloudPolyData->Modified();
//     }

//     // 6. 配置渲染管线
//     if (currentColorMode == ColorMode::RadialVelocity) {
//         pointCloudMapper->SetColorModeToMapScalars();
//         pointCloudMapper->ScalarVisibilityOn(); // 确保标量可见性开启
//         pointCloudMapper->SelectColorArray("radVelAbs");
//         pointCloudMapper->SetLookupTable(radarLUT);
//         pointCloudMapper->SetScalarRange(0, 50);

//         if (scalarBarActor) {
//             scalarBarActor->SetVisibility(true);
//             scalarBarActor->Modified();
//         }
//     } else {
//         // 直接渲染 pt.r, pt.g, pt.b
//         pointCloudMapper->SetColorModeToDirectScalars();
//         if (scalarBarActor) scalarBarActor->SetVisibility(false);
//     }

//     pointCloudPolyData->ComputeBounds();

//         // 2. 🌟 关键：显式更新 Points 的修改时间
//         // 虽然你改了 pData，但必须显式调用这个，Picker 才会意识到“地图”失效了
//         if (pointCloudPolyData->GetPoints()) {
//             pointCloudPolyData->GetPoints()->Modified();
//         }

//         // 3. 强制触发 Mapper 的抽象更新（不需要 Render 两次）
//         // 这会引导 VTK 重新检查内部数据一致性
//         pointCloudMapper->Update();

//         // 4. 通知 Picker 缓存失效
//         if (pointPicker) {
//             // 由于 vtkPointPicker 没有 GetLocator，我们通过调用 Modified
//             // 配合前面 Points->Modified()，会强制它在下一次 Pick 时重新搜索点
//             pointPicker->Modified();
//         }
//         if (pointCloudPolyData->GetNumberOfPoints() != pointCloudPolyData->GetVerts()->GetNumberOfCells()) {
//             vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
//             for (vtkIdType i = 0; i < numPoints; ++i) {
//                 vertices->InsertNextCell(1, &i);
//             }
//             pointCloudPolyData->SetVerts(vertices);
//         }
//     // 刷新渲染
//     pointCloudActor->SetVisibility(numPoints > 0);
//     ui->qvtkWidget->renderWindow()->Render();
// }
void MainWindow::on_saveButton_clicked()
{
    try {
           if (!satecloud || satecloud->points.empty()) {
               QMessageBox::warning(this, "保存失败", "当前无有效点云数据！");
               return;
           }

           satecloud->width  = static_cast<uint32_t>(satecloud->points.size());
                   satecloud->height = 1;
                   satecloud->is_dense = false;

           QString fileName = QFileDialog::getSaveFileName(this, "保存点云文件", "", "PCD 文件 (*.pcd)");
           if (fileName.isEmpty())
               return;



           // Windows下UTF-8路径可能导致 PCL 出错，最好用临时英文路径
           QString tempPath = QDir::temp().filePath("temp_save.pcd");
           std::string temp_utf8 = tempPath.toUtf8().constData();

           int ret = pcl::io::savePCDFileASCII(temp_utf8, *satecloud);
           if (ret == 0) {
               QFile::rename(tempPath, fileName);
               QMessageBox::information(this, "保存成功", "点云已保存至:\n" + fileName);
           } else {
               QMessageBox::critical(this, "保存失败", "无法保存点云文件！");
           }
       } catch (const std::exception &e) {
           QMessageBox::critical(this, "保存失败", QString("异常：%1").arg(e.what()));
       } catch (...) {
           QMessageBox::critical(this, "保存失败", "未知异常！");
       }
}


void MainWindow::on_loadButton_clicked()
{
    clearHighlightedPoints();

    timer->stop();
    multiFrames.clear(); // 清空多帧列表

    frames8x8.clear();   // 确保清理其他模式（如8x8）的帧数据

        // 1. 安全清空主点云指针 (PCL 数据)
        if (satecloud) {
            satecloud.reset();
        }

        // 2. 移除 VTK 渲染 Actor (屏幕显示)
        if (pointCloudActor) {
            mainRenderer->RemoveActor(pointCloudActor);
            pointCloudActor = nullptr; // 重置指针，确保 displayPointCloud() 重建管线
        }


    QString fileName = QFileDialog::getOpenFileName(this, "加载点云文件", "", "PCD 文件 (*.pcd)");
    if (fileName.isEmpty())
        return;

    QString tempPath = QDir::temp().filePath("temp_load.pcd");

        // 使用读写方式安全复制文件
        QFile src(fileName);
        if (!src.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "加载失败", "无法打开原始 PCD 文件！");
            return;
        }

        QFile dst(tempPath);
        if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::critical(this, "加载失败", "无法创建临时 PCD 文件！");
            return;
        }

        while (!src.atEnd()) {
            QByteArray chunk = src.read(4096);
            if (dst.write(chunk) != chunk.size()) {
                QMessageBox::critical(this, "加载失败", "写入临时文件失败！");
                return;
            }
        }
        src.close();
        dst.close();



    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>  loadedCloud(new pcl::PointCloud<PointXYZRGBWithProperties>);

    int ret = pcl::io::loadPCDFile<PointXYZRGBWithProperties>(tempPath.toLocal8Bit().toStdString(), *loadedCloud);
    if (ret == -1) {
        QMessageBox::critical(this, "加载失败", "无法加载 PCD 文件！");
        return;
    }
    const int currentFrameIndex = 0;

    for (size_t i = 0; i < loadedCloud->points.size(); ++i) {
            auto& pt = loadedCloud->points[i];
            pt.frameIndex = currentFrameIndex;
            pt.originalIndex = static_cast<int>(i); // 使用在帧内的索引作为 originalIndex
        }
    multiFrames.push_back(loadedCloud);
    satecloud = loadedCloud;

    if (!satecloud->empty()) {
        // 使用 colorizer 对 satecloud (即 loadedCloud) 进行着色
        // 注意：你需要确保 colorizer 内部能够访问到用于颜色映射的属性极值（例如速度的最大最小值）。
        colorizer.colorizePointCloud(satecloud, currentColorMode);
    }

    isFirstFrame = true;  // 重置视角
    displayPointCloud();

    if (loadedFilesDock) {
            loadedFilesDock->updateOnePCD(QFileInfo(fileName).fileName());
        }
}


void MainWindow::saveMultiFrameBin(const std::string& filename,
                       int startFrame,
                       int endFrame,
                       const std::vector<pointandtimedata>& frames)
{
    if (startFrame < 0 || endFrame >= static_cast<int>(frames.size()) || startFrame > endFrame) {
        throw std::runtime_error("Invalid frame range.");
    }

    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    uint64_t frameCount = static_cast<uint64_t>(endFrame - startFrame + 1);
    ofs.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));

    for (int i = startFrame; i <= endFrame; ++i) {
        const auto& p = frames[i];

        uint64_t indexNo = static_cast<uint64_t>(p.indexno);
        uint64_t startTimeNs = p.start_time_ns;
        uint64_t timestampNs = p.timestamp_ns;

        ofs.write(reinterpret_cast<const char*>(&indexNo), sizeof(indexNo));
        ofs.write(reinterpret_cast<const char*>(&startTimeNs), sizeof(startTimeNs));
        ofs.write(reinterpret_cast<const char*>(&timestampNs), sizeof(timestampNs));

        // 保存 dblArray
        uint64_t dblVecCount = static_cast<uint64_t>(p.Radar4T4Point.dblArray.size());
        ofs.write(reinterpret_cast<const char*>(&dblVecCount), sizeof(dblVecCount));
        for (const auto& vec : p.Radar4T4Point.dblArray) {
            uint64_t vlen = static_cast<uint64_t>(vec.size());
            ofs.write(reinterpret_cast<const char*>(&vlen), sizeof(vlen));
            ofs.write(reinterpret_cast<const char*>(vec.data()), vlen * sizeof(double));
        }
    }
}


void MainWindow::on_saveMultiFrameButton_clicked()
{
    if (frames.empty()) {
        QMessageBox::warning(this, "错误", "没有可保存的帧！");
        return;
    }

    FrameRangeDialog dlg(frames.size() - 1, this);
    if (dlg.exec() == QDialog::Accepted) {
        int startFrame = dlg.startFrame();
        int endFrame   = dlg.endFrame();
        QString outputFile = dlg.saveFilePath();

        if (outputFile.isEmpty()) {
            QMessageBox::warning(this, "提示", "请选择保存路径！");
            return;
        }

        try {
            saveMultiFrameBin(outputFile.toStdString(), startFrame, endFrame, frames);
            QMessageBox::information(this, "成功", "多帧文件保存完成！");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "错误", QString("保存失败: %1").arg(e.what()));
            qDebug() << "保存失败：" << e.what();
        }
    }




}

void MainWindow::clearMultiFrames()
{
    multiFrames.clear();
}


// 加载多帧BIN示例，加载后自动显示第一帧
void MainWindow::loadMultiFrameBin(const QString& filename)
{

    clearMultiFrames();
    try {
            frames = loadPointsFromBin(filename.toStdString());
        } catch (const std::exception& e) {
            QMessageBox::critical(
                this,
                "加载失败",
                QString("该 BIN 文件不是有效的 4x4 点云文件。\n\n原因：%1").arg(e.what())
            );
            return;
        }


    if (frames.empty()) {
        QMessageBox::warning(this, "加载失败", "文件没有有效帧！");
        return;
    }



    int frameCounter = 0;

    for (const auto& frame : frames) {
        //ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frame.start_time_ns)));


        std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cloudFrame(
            new pcl::PointCloud<PointXYZRGBWithProperties>
            );

        size_t pointCount = frame.Radar4T4Point.dblArray[0].size();
        cloudFrame->points.resize(pointCount);

        // double minAbs = std::numeric_limits<double>::max();
        // double maxAbs = std::numeric_limits<double>::lowest();
        // for (size_t i = 0; i < pointCount; ++i) {

        //     if(frame.Radar4T4Point.dblArray[15][i]>0){
        //         double v = std::abs(frame.Radar4T4Point.dblArray[8][i]);
        //         minAbs = std::min(minAbs, v);
        //         maxAbs = std::max(maxAbs, v);
        //     }
        // }




        for (size_t i = 0; i < pointCount; ++i) {
            PointXYZRGBWithProperties& pt = cloudFrame->points[i];



            pt.range = frame.Radar4T4Point.dblArray[0][i];
            pt.dopplerSpeed  = frame.Radar4T4Point.dblArray[1][i];
            pt.powerdB  = frame.Radar4T4Point.dblArray[2][i];
            pt.SNRdB = frame.Radar4T4Point.dblArray[3][i];
            pt.Q_azi = frame.Radar4T4Point.dblArray[4][i];
            pt.Q_ele = frame.Radar4T4Point.dblArray[5][i];
            pt.azimuthAng = frame.Radar4T4Point.dblArray[6][i];
            pt.eleAng = frame.Radar4T4Point.dblArray[7][i];
            pt.radVelAbs = frame.Radar4T4Point.dblArray[8][i];
            pt.x = frame.Radar4T4Point.dblArray[9][i];
            pt.y = frame.Radar4T4Point.dblArray[10][i];
            pt.z = frame.Radar4T4Point.dblArray[11][i];
            pt.detValid=frame.Radar4T4Point.dblArray[15][i];
            pt.rcsdB = frame.Radar4T4Point.dblArray[16][i];


            pt.originalIndex = static_cast<int>(i);

            pt.frameIndex = frameCounter;
            //qDebug()<<pt.radVelAbs;
            // 颜色你可以自定义  蓝色逐渐变亮到橙色

            // if (std::abs(pt.radVelAbs) <= 1.0) {
            //     // 小于1 显示绿色
            //     pt.r = 0;
            //     pt.g = 200;
            //     pt.b = 0;
            // } else {
            // double t = (std::abs(pt.radVelAbs) - minAbs) / (maxAbs - minAbs + 1e-9);
            // //t = std::min(std::max(t, 0.0), 1.0);
            // t = std::clamp(t, 0.0, 1.0);
            //  t = t * t;
            // //t = std::sqrt(t);
            //  /*
            //  pt.r = static_cast<unsigned char>((1.0 - t) * 0   + t * 230);
            //  pt.g = static_cast<unsigned char>((1.0 - t) * 114 + t * 159);
            //  pt.b = static_cast<unsigned char>((1.0 - t) * 178 + t * 0);
            // */
            //  pt.r = static_cast<unsigned char>(255 * t);  // 红色增加到黄色
            //  pt.g = static_cast<unsigned char>(165 * t);  // 绿色增加到黄色
            //  pt.b = static_cast<unsigned char>(255 * (1.0 - t)); // 蓝色减少
            // /*pt.r = 255;
            // pt.g = 255;
            // pt.b = 255;*/
            // }

        }

        colorizer.colorizePointCloud(cloudFrame, currentColorMode);
        cloudFrame->width = pointCount;
        cloudFrame->height = 1;
        cloudFrame->is_dense = false;

        multiFrames.push_back(cloudFrame);
        frameCounter++;
    }


    if (!multiFrames.empty()) {
        satecloud = multiFrames[0];
        isFirstFrame = true;
        displayPointCloud();
        frameBuffer.clear();
        lastAccumulatedCloud = nullptr;

        QMessageBox::information(this, "加载成功",
                                 QString("加载了 %1 帧点云，显示第1帧").arg(multiFrames.size()));

        ui->frameSlider->setMinimum(0);
        ui->frameSlider->setMaximum(static_cast<int>(multiFrames.size()) - 1);
        ui->frameSlider->setValue(0);
        ui->frameSlider->setSingleStep(1);
        ui->frameSlider->setPageStep(1);
        currentFrameIndex = 0;
        isPlaying = false;

        ui->pointstarttime->setText(QString::fromStdString(convertTimestamp(frames[0].start_time_ns)));
        ui->pointruntime->setText(QString::number(frames[currentFrameIndex].indexno ));
        double seconds = frames[multiFrames.size()-1].timestamp_ns / 1000000000.0;
        QString str = QString::number(seconds, 'f', 3)+" s"; // 保留3位小数
        ui->pointendtime->setText(str);
    }
}


void MainWindow::on_loadMultiFrameButton_clicked()
{
    clearHighlightedPoints();
    if (cumulativeCloud) cumulativeCloud->clear();
    if (pointCloudActor) {
        mainRenderer->RemoveActor(pointCloudActor);
        pointCloudActor = nullptr; // 强制重置 Actor
    }
    // 停止所有播放
    if (playbackTimer->isActive()) {
        playbackTimer->stop();
    }
    // 停止视频播放
    if (m_player) {
        m_player->stop();
    }

    timer->stop();

    frameBuffer.clear();            // ⬅️ 清除所有积累的旧帧！
    lastAccumulatedCloud.reset();   // ⬅️ 清除积累追踪器
    isFirstFrame = true;


    isRadar8x8Mode = false;         // ⬅️ 设置为 4x4/MultiFrames 模式
    rdMapPlot->clearPlottables();
    rdMapPlot->replot();

    QString filename = QFileDialog::getOpenFileName(this, "加载多帧点云文件", "", "Bin Files (*.bin)");
    if (filename.isEmpty()) return;

    size_t total_frames = 0;

  try {
    if (ui->signal4_4->isChecked()) {
        // 清理互斥数据
        frames8x8.clear();

        // 设置模式
        isRadar8x8Mode = false;
        // 清理 8x8 特有 UI
        rdMapPlot->clearPlottables();
        rdMapPlot->replot();

        loadMultiFrameBin(filename);  // ⬅️ 调用你已有的函数


        total_frames = multiFrames.size();

        if (multiFrames.empty()) {
            QMessageBox::warning(
                this,
                "加载失败",
                "该 BIN 文件不是有效的 4x4 点云数据，或文件已损坏。"
            );
            return;
        }


        pcMinMax = calculateMultiFrameMinMax(multiFrames);
        initSlidersFromPointCloud(pcMinMax.get());
        currentFrameIndex = 0;



        if (!multiFrames.empty()) {
            satecloud = multiFrames[0];  // 显示第一帧
            displayPointCloud();
        }

        if (loadedFilesDock) {
                    loadedFilesDock->updatePointBin(QFileInfo(filename).fileName());
                }

                QMessageBox::information(this, "加载成功",
                                         QString("加载了 %1 帧点云").arg(total_frames));
    }else if (ui->signal8_8->isChecked()) {
        // 清理互斥数据
        multiFrames.clear();

        // 设置模式
        isRadar8x8Mode = true;

        // 加载数据
        // 假设您有一个加载 8x8 BIN 的函数
        frames8x8 = Load8T8VectorFromBin(filename.toStdString());
        total_frames = frames8x8.size();

        if (frames8x8.empty()) {
            QMessageBox::warning(
                this,
                "加载失败",
                "该 BIN 文件不是有效的 8x8 点云数据，或文件已损坏。"
            );
            return;
        }

        if (!frames8x8.empty()) {
            for (int i = 0; i < frames8x8.size(); ++i) {
                        auto cloud = frames8x8[i].Radar8T8Point.cloud;
                        // 确保 cloud 存在且非空
                        if (cloud && !cloud->points.empty()) {
                            // 假设底层已设置 frameIndex == i
                            multiFrames.push_back(cloud);
                        }
                    }
            if (multiFrames.empty()) {
                         QMessageBox::warning(this, "加载失败", "8x8 BIN文件加载成功，但所有帧的点云数据均为空。");
                         return;
                    }

            ui->frameSlider->setMinimum(0);
            ui->frameSlider->setMaximum(static_cast<int>(total_frames) - 1);
            ui->frameSlider->setValue(0);
            ui->frameSlider->setSingleStep(1);
            ui->frameSlider->setPageStep(1);

            satecloud = multiFrames[0];
            pcMinMax = calculate8T8MinMax(frames8x8); // 假设有这个函数
            initSlidersFromPointCloud(pcMinMax.get());
            // 8x8 模式特有的 RD Map 初始化
            rdMapPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            showRDMap(rdMapPlot, frames8x8[0].Radar8T8Point.rdmap.data());
             displayPointCloud(); // 如果没有视频就直接显示
             if (m_player) {
                 if (m_player->mediaStatus() == QMediaPlayer::LoadedMedia ||
                     m_player->mediaStatus() == QMediaPlayer::BufferedMedia) {

                     qint64 pos = m_player->position(); // 获取当前视频时间

                     // ⚡ 强制刷新视频帧
                     isUpdatingFromVideo = true;
                     m_player->blockSignals(true);      // 避免触发 positionChanged 循环
                     m_player->setPosition(pos);        // 更新视频位置
                     m_player->pause();                 // 必须 pause 才能刷新画面
                     m_player->blockSignals(false);
                     isUpdatingFromVideo = false;

                     // ⚡ 更新点云显示到当前视频帧
                     updatePointCloudFromVideo(pos);
                 }
             }

             if (loadedFilesDock) {
                         loadedFilesDock->updatePointBin(QFileInfo(filename).fileName());
                     }

            QMessageBox::information(this, "加载成功",
                                     QString("加载了8x8 %1 帧点云，显示第1帧").arg(total_frames));
        } else {
            // 如果 8x8 加载失败，确保 RD Map 清空
            rdMapPlot->clearPlottables();
            rdMapPlot->replot();
        }
    }

    } catch (const std::exception& e) {
           QMessageBox::warning(this, "加载失败", QString("加载 BIN 文件失败：%1").arg(e.what()));
           return;
       } catch (...) {
           QMessageBox::warning(this, "加载失败", "加载 BIN 文件出现未知错误！");
           return;
       }
}

void MainWindow::on_playPauseButton_clicked()
{


    if (frames8x8.empty() && multiFrames.empty()) return;
    bool isBinOfflineMode = (isRadar8x8Mode && !frames8x8.empty() && frames8x8[0].start_time_ns == 0);
    // 如果媒体已加载或正在播放
    // ==========================================
        // 视频播放器控制 (AVI 视频)
        // ==========================================
        if (isBinOfflineMode) {
            // 🌟【针对离线 BIN 模式的专属安全隔离】：
            // 视频自己绝对不调用 play()，永远保持暂停挂起状态，由后面的 Timer 强行驱动 setPosition
            if (m_player->state() == QMediaPlayer::PlayingState) {
                m_player->pause();
            }
        }
        else {
            // 🌟【其他所有历史模式】：原封不动，完美保留你以前的所有逻辑！
            if (m_player->state() == QMediaPlayer::PlayingState) {
                m_player->pause();
                ui->playPauseButton->setText("Play");
            } else {
                if (m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
                    m_player->setPosition(0);
                    m_player->play();
                    ui->playPauseButton->setText("Pause");
                }
                if (m_player->mediaStatus() == QMediaPlayer::LoadedMedia ||
                    m_player->mediaStatus() == QMediaPlayer::BufferedMedia ||
                    m_player->mediaStatus() == QMediaPlayer::StalledMedia) {
                    m_player->play();
                    ui->playPauseButton->setText("Pause");
                }
            }
        }



    if (isPlaying) {
        playbackTimer->stop();
        isPlaying = false;
        ui->playPauseButton->setText("Play");

    } else {
        isPlaying = true;
        ui->playPauseButton->setText("Pause");
       // playbackTimer->start(300);  // 每帧300ms，可调
        // 如果已经播放到最后一帧，则从头开始
        // currentFrameIndex = 0; // 重置索引
        // ✅ 8x8 或 4x4，统一触发定时器

        // 🌟【核心改动点】：精准对齐不连续帧的边界
                if (isBinOfflineMode) {

                    if (currentFrameIndex > ui->frameSlider->maximum()) {
                                    currentFrameIndex = ui->frameSlider->minimum();
                                }
                                if (m_player) m_player->pause(); // 确保视频挂起待命
                                playbackTimer->start(50);


                    if (!m_validFrameIds.empty()) {
                        int absoluteMaxFrame = *m_validFrameIds.rbegin(); // 最大的硬件帧号 (例如 180)
                        int absoluteMinFrame = *m_validFrameIds.begin();  // 最小的硬件帧号 (例如 13)

                        // 如果当前播放指针已经顶天了，或者越界了，重新播放时立刻空降回“数据起点”
                        if (currentFrameIndex >= absoluteMaxFrame) {
                            currentFrameIndex = absoluteMinFrame;
                        }

                        playbackTimer->start(1); // 立即激活后台 Timeout 逻辑（在那里会执行 lower_bound 闪现）
                    }
                }
                else {


        size_t total_frames = isRadar8x8Mode ? frames8x8.size() : multiFrames.size();
                if ((isRadar8x8Mode && !frames8x8.empty()) || (!isRadar8x8Mode && !multiFrames.empty())) {
                    // 如果已经播放到最后一帧，则从头开始
                    if (currentFrameIndex >= total_frames) {
                        currentFrameIndex = 0;

                    }
                    playbackTimer->start(1); // 立即触发 Lambda
                }
        }
        // satecloud = multiFrames[currentFrameIndex];
        // displayPointCloud();
        // ui->frameSlider->setValue(currentFrameIndex);
        // ui->pointruntime->setText(QString::number(frames[currentFrameIndex].indexno + 1));



        // if (currentFrameIndex < multiFrames.size() - 1) {
        //     long long current_time_ns = frames[currentFrameIndex].timestamp_ns;
        //     long long next_time_ns = frames[currentFrameIndex + 1].timestamp_ns;

        //     long long diff_ns = next_time_ns-current_time_ns;

        //     // 将纳秒转换为毫秒
        //     int delay_ms = static_cast<int>(diff_ns / 1000000);
        //     if (delay_ms < 1) {
        //         delay_ms = 1; // 确保至少有1毫秒的延迟，避免卡顿
        //     }

        //     playbackTimer->setSingleShot(true); // 确保定时器只触发一次
        //     playbackTimer->start(delay_ms);
        // }
    }


    ui->FramePlayButton->setIcon(
            isPlaying ? style()->standardIcon(QStyle::SP_MediaPause)
                      : style()->standardIcon(QStyle::SP_MediaPlay)
        );

}


void MainWindow::on_checkBoxSelectMode_toggled(bool checked)
{

    selectionModeEnabled = checked;

    if (!selectionModeEnabled){

        // 1. 清理 UI
        ui->comboBoxSelectedPoints->clear();
        ui->labelPointInfo->setText("未选中点");

        // 2. 🌟 关键修正：清理多帧数据状态 🌟
        // 🎯 必须清理用于多帧键控的集合
        if (!selectedPointKeys.isEmpty()) {
            selectedPointKeys.clear();
        }

        // 兼容旧的单帧 ID 集合（如果仍在其他地方使用）
        // 🚨 警告：如果您已经完全迁移到 selectedPointKeys，则可以移除下一行
        // selectedPointIds.clear();


        // 3. 清理 VTK 几何体
        if (highlightedPoints) {
            highlightedPoints->Reset(); // 清空 VTK Points 对象中的所有坐标
            highlightedPoints->Modified(); // 通知 VTK 数据已更改
            // 不需要将 highlightedPoints 设置为 nullptr，Reset() 就足够了，
            // 除非您在 addPointToSelectionnew 中是检查 nullptr 来决定是否创建新的 vtkPoints
        }

        // 4. 移除 Actor
        if (highlightedActor) {
            mainRenderer->RemoveActor(highlightedActor);
            highlightedActor = nullptr; // 清空指针，让 addPointToSelectionnew 重新创建
        }

        // 移除平面 Actor
        if (existingPlaneActor) {
            mainRenderer->RemoveActor(existingPlaneActor);
            existingPlaneActor = nullptr; // 假设您也想清空平面 Actor
        }

        // 5. 强制渲染
        ui->qvtkWidget->renderWindow()->Render();
    }
}
/*
void MainWindow::on_frameSlider_valueChanged(int value)
{
    if (value >= 0 && value < static_cast<int>(multiFrames.size())) {
        currentFrameIndex = value;
        satecloud = multiFrames[currentFrameIndex];
        frameBuffer.clear();
        lastAccumulatedCloud = nullptr;

        displayPointCloud();
    }
}

*/
// bool MainWindow::eventFilter(QObject* obj, QEvent* event)
// {

//     if (obj == ui->qvtkWidget) {
//         QMouseEvent* me = static_cast<QMouseEvent*>(event);
//         // 鼠标按下，保存起点
//         if (selectionModeEnabled) {
//             if (event->type() == QEvent::MouseButtonPress) {
//                 QMouseEvent* me = static_cast<QMouseEvent*>(event);
//                 //if (me->button() == Qt::LeftButton && (me->modifiers() & Qt::ControlModifier))
//                 if (me->button() == Qt::LeftButton) {
//                     pressX = me->pos().x();
//                     pressY = ui->qvtkWidget->height() - me->pos().y();
//                     //qDebug()<<pressY;
//                     return true;
//                 }
//             }

//             // 鼠标释放，获取终点并框选
//             if (event->type() == QEvent::MouseButtonRelease) {
//                 QMouseEvent* me = static_cast<QMouseEvent*>(event);
//                 //if (me->button() == Qt::LeftButton && (me->modifiers() & Qt::ControlModifier))
//                 if (me->button() == Qt::LeftButton) {
//                     int releaseX = me->pos().x();
//                     int releaseY = ui->qvtkWidget->height() - me->pos().y();


//                     //qDebug()<<"pressY";
//                     doAreaPick(pressX, pressY, releaseX, releaseY); // 你自己的框选函数



//                     return true;
//                 }
//             }
//         }
//         // 现有的双击事件处理
//         if (event->type() == QEvent::MouseButtonDblClick) {
//             QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
//             if (mouseEvent->button() == Qt::LeftButton) {
//                 int x = mouseEvent->pos().x();
//                 int y = mouseEvent->pos().y();
//                 // 由于VTK和Qt的Y轴方向相反，转换坐标
//                 int vtkY = ui->qvtkWidget->height() - y;

//                 if (!pointPicker) return true;
//                 /*
//                 vtkSmartPointer<vtkPointPicker> pointPickernew = vtkSmartPointer<vtkPointPicker>::New();
//                 pointPickernew->SetTolerance(0.01);
//                 pointPickernew->PickFromListOn();
//                 pointPickernew->AddPickList(pointCloudActor);  // 只拾取点云 actor
//                 */
//                 if (pointPicker->Pick(x, vtkY, 0, mainRenderer)) {
//                     vtkIdType pointId = pointPicker->GetPointId();



//                     if (pointId >= 0) {
//                         handlePickedPoint(satecloud, pointId);
//                     } else {
//                         QMessageBox::information(this, "提示", "未拾取到点");
//                     }
//                 }

//                 return true; // 事件处理完毕
//             }
//         }
//     }

//     if (obj == ui->qvtkWidget && event->type() == QEvent::Resize) {
//         updateInfoPanelPosition();
//     }

//     return QMainWindow::eventFilter(obj, event);






// }


void MainWindow::handlePickedPointnew(vtkIdType pickedPointId)
{
    // 1. 获取拾取到的 Actor 和 PolyData
    vtkActor* hitActor = pointPicker->GetActor();
    if (!hitActor) return;

    vtkPolyData* polyData = vtkPolyData::SafeDownCast(hitActor->GetMapper()->GetInputDataObject(0, 0));
    if (!polyData) return;

    // 2. 获取存储在 PolyData 中的 FrameID 和 OriginalID 数组
    vtkIntArray* originalIdArray = vtkIntArray::SafeDownCast(
        polyData->GetPointData()->GetArray("OriginalID"));
    vtkIntArray* frameIdArray = vtkIntArray::SafeDownCast(
        polyData->GetPointData()->GetArray("FrameID"));

    // 检查数组是否存在且索引有效
    if (originalIdArray && frameIdArray &&
        pickedPointId >= 0 && pickedPointId < originalIdArray->GetNumberOfTuples()) {

        // 3. 提取正确的索引和帧号
        int originalPointId = originalIdArray->GetValue(pickedPointId);
        int frameIndex = frameIdArray->GetValue(pickedPointId);

        // 4. 调用新的选中逻辑 (必须支持 FrameIndex)
        // **注意:** 这要求您的 addPointToSelection 签名和内部逻辑支持 (FrameIndex, OriginalPointId)
        addPointToSelectionnew(frameIndex, originalPointId);

    } else {
        // 如果无法获取映射数据（例如拾取到了非点云的 Actor，或数据缺失）
        QMessageBox::information(this, "提示", "拾取成功，但无法获取点云映射信息。");
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{

    // 确保对象是 qvtkWidget
    if (obj != ui->qvtkWidget) {
        return QMainWindow::eventFilter(obj, event);
    }



    QMouseEvent* me = static_cast<QMouseEvent*>(event);
    vtkRenderWindowInteractor *interactor = ui->qvtkWidget->renderWindow()->GetInteractor();

    // --- 通用修饰键状态 ---
    // 只有在 MouseEvent 或 WheelEvent 发生时才有效
    int ctrl = me ? (me->modifiers() & Qt::ControlModifier ? 1 : 0) : 0;
    int shift = me ? (me->modifiers() & Qt::ShiftModifier ? 1 : 0) : 0;



    // ==========================================================
    // 1. 🌟 优先处理双击事件 (您的拾取逻辑) 🌟
    // ==========================================================
    if (event->type() == QEvent::MouseButtonDblClick) {

        if (me && me->button() == Qt::LeftButton) {
            int x = me->pos().x();
            int y = me->pos().y();
            // 由于VTK和Qt的Y轴方向相反，转换坐标
            int vtkY = ui->qvtkWidget->height() - y;


            if (!pointPicker) return true;

            if (pointPicker->Pick(x, vtkY, 0, mainRenderer)) {
                vtkIdType pointId = pointPicker->GetPointId();

                if (pointId >= 0) {
                    // 请根据您的最终函数签名调用，例如：
                    // handlePickedPoint(satecloud, pointId);
                    handlePickedPointnew(pointId);
                    // 或修正后的：
                    // handlePickedPoint(pointId);
                } else {
                    QMessageBox::information(this, "提示", "未拾取到点");
                }
            }
            ui->qvtkWidget->renderWindow()->Render();
            return true; // 消费双击事件
        }
    }


    // ==========================================================
    // 2. 框选模式下的鼠标按下/释放逻辑 (阻止相机交互)
    // ==========================================================
    if (selectionModeEnabled) {

        if (event->type() == QEvent::MouseButtonPress) {
            if (me && me->button() == Qt::LeftButton) {
                // 保存起点
                pressX = me->pos().x();
                pressY = ui->qvtkWidget->height() - me->pos().y();
                return true; // 拦截事件，阻止旋转
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            if (me && me->button() == Qt::LeftButton) {
                // 执行框选
                int releaseX = me->pos().x();
                int releaseY = ui->qvtkWidget->height() - me->pos().y();
                doAreaPicknew(pressX, pressY, releaseX, releaseY);
                return true; // 拦截事件，阻止旋转
            }
        }
    }


    // ==========================================================
    // 3. 🌟 手动 Interactor 逻辑 (仅在非选择模式下接管相机) 🌟
    // ==========================================================
    if (!selectionModeEnabled) {


        // ------------------------------------
        // A. 鼠标按下事件 (Press): 决定交互的开始
        // ------------------------------------
        if (event->type() == QEvent::MouseButtonPress) {

            char button_key = '0'; // VTK 按钮代码

            if (me->button() == Qt::LeftButton) {
                button_key = '1'; // 左键 (旋转)
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, button_key, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
                return false;
            } else if (me->button() == Qt::MiddleButton || (me->button() == Qt::LeftButton && me->modifiers() & Qt::ControlModifier)) {
                button_key = '2'; // 中键 (平移)
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, button_key, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::MiddleButtonPressEvent, nullptr);
                return false;
            } else if (me->button() == Qt::RightButton) {
                button_key = '3'; // 右键 (缩放)
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, button_key, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
                return false;
            }
        }

        // ------------------------------------
        // B. 鼠标移动事件 (Move): 驱动交互
        // ------------------------------------
        else if (event->type() == QEvent::MouseMove) {
            // --- 🌟 新增：鼠标悬停提示逻辑 🌟 ---
            if (me && !selectionModeEnabled) {

                int x = me->pos().x();
                int y = me->pos().y();
                int vtkY = ui->qvtkWidget->height() - y;

                // 仅在鼠标不按住任何按钮时（即纯粹的悬停）执行拾取，
                // 以避免干扰相机旋转/平移时的性能。
                if (me->buttons() == Qt::NoButton && pointPicker && mainRenderer) {

                    // 尝试拾取点云（使用较小的容忍度）
                    if (pointPicker->Pick(x, vtkY, 0, mainRenderer)) {
                        vtkIdType pickedPointId = pointPicker->GetPointId();
                        vtkActor* hitActor = pointPicker->GetActor();

                        if (pickedPointId >= 0 && hitActor) {
                            // 访问你的自定义点云数据结构
                            // ** 这是根据你的代码行修改的关键部分 **
                            vtkPolyData* polyData = vtkPolyData::SafeDownCast(hitActor->GetMapper()->GetInputDataObject(0, 0));

                            if (!polyData) {
                                QToolTip::hideText(); // 无法获取数据，隐藏提示
                                return false;
                            }

                            // 🌟 关键修正：从 PolyData 的 PointData 数组中读取所有属性 🌟

                            // 1. 获取 X, Y, Z 坐标
                            double xyz[3];
                            polyData->GetPoint(pickedPointId, xyz);

                            // 2. 获取其他 PCL 属性（数组名称必须与 convertmydataToVTKPolyData 中设置的一致）
                            // 使用 GetArray() 获取数组，然后用 GetTuple1() 或 GetValue() 读取值

                            auto getDoubleValue = [&](const char* arrayName) -> double {
                                vtkDataArray* arr = polyData->GetPointData()->GetArray(arrayName);
                                return (arr && pickedPointId < arr->GetNumberOfTuples()) ? arr->GetTuple1(pickedPointId) : 0.0;
                            };

                            double range = getDoubleValue("Range");
                            double dopplerSpeed = getDoubleValue("DopplerSpeed");
                            double powerdB = getDoubleValue("PowerdB");
                            double SNRdB = getDoubleValue("SNRdB");
                            double Q_azi = getDoubleValue("Q_azi");
                            double Q_ele = getDoubleValue("Q_ele");
                            double azimuthAng = getDoubleValue("AzimuthAng"); // 注意大小写
                            double eleAng = getDoubleValue("EleAng");         // 注意大小写
                            double radVelAbs = getDoubleValue("radVelAbs");
                            double detValid = getDoubleValue("detValid");
                            double rcsdB = getDoubleValue("RcsdB");           // 注意大小写
                            int originalId = static_cast<int>(getDoubleValue("OriginalID"));
                            int rangebin = getDoubleValue("Rangebin");
                            int velbin = getDoubleValue("Velbin");


                            // 格式化并显示提示信息
                            // 假设标签最大宽度 (Item 左对齐)
                            const int LABEL_WIDTH = 15;
                            // 假设数值字段宽度 (Value 左对齐)
                            const int VALUE_WIDTH = 10;
                            // 小数点位数
                            const int PRECISION = 2;

                            QString toolTipText = QString(
                                                      // 格式字符串不变，仍使用 %1 %2 组合
                                                      "%1%2\n"
                                                      "%3%4\n"
                                                      "%5%6\n"
                                                      "%7%8\n"
                                                      "%9%10\n"
                                                      "%11%12\n"
                                                      "%13%14\n"
                                                      "%15%16\n"
                                                      "%17%18\n"
                                                      "%19%20\n"
                                                      "%21%22\n"
                                                      "%23%24\n"
                                                      "%25%26\n"
                                                      "%27%28\n"
                                                      "%29%30\n"
                                                      "%31%32\n"
                                                      "%33%34"
                                                      )
                                                      // --- Target ID ---
                                                      .arg(QString("Target ID:").leftJustified(LABEL_WIDTH, ' ')) // %1 (Item 左对齐)
                                                      // 格式化数值后，再用 leftJustified() 实现左对齐
                                                      .arg(QString::number(pickedPointId).rightJustified(VALUE_WIDTH, ' ')) // %2 (Value 左对齐)

                                                      // --- X (m) ---
                                                      .arg(QString("X (m):").leftJustified(LABEL_WIDTH, ' ')) // %3
                                                      .arg(QString::number(xyz[0], 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %4

                                                      // --- Y (m) ---
                                                      .arg(QString("Y (m):").leftJustified(LABEL_WIDTH, ' ')) // %5
                                                      .arg(QString::number(xyz[1], 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %6

                                                      // --- Z (m) ---
                                                      .arg(QString("Z (m):").leftJustified(LABEL_WIDTH, ' ')) // %7
                                                      .arg(QString::number(xyz[2], 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %8

                                                      // --- range ---
                                                      .arg(QString("range:").leftJustified(LABEL_WIDTH, ' ')) // %9
                                                      .arg(QString::number(range, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %10

                                                      // --- dopplerSpeed ---
                                                      .arg(QString("dopplerSpeed:").leftJustified(LABEL_WIDTH, ' ')) // %11
                                                      .arg(QString::number(dopplerSpeed, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %12

                                                      // --- powerdB ---
                                                      .arg(QString("powerdB:").leftJustified(LABEL_WIDTH, ' ')) // %13
                                                      .arg(QString::number(powerdB, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %14

                                                      // --- SNRdB ---
                                                      .arg(QString("SNRdB:").leftJustified(LABEL_WIDTH, ' ')) // %15
                                                      .arg(QString::number(SNRdB, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %16

                                                      // --- Q_azi ---
                                                      .arg(QString("Q_azi:").leftJustified(LABEL_WIDTH, ' ')) // %17
                                                      .arg(QString::number(Q_azi, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %18

                                                      // --- Q_ele ---
                                                      .arg(QString("Q_ele:").leftJustified(LABEL_WIDTH, ' ')) // %19
                                                      .arg(QString::number(Q_ele, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %20

                                                      // --- azimuthAng ---
                                                      .arg(QString("azimuthAng:").leftJustified(LABEL_WIDTH, ' ')) // %21
                                                      .arg(QString::number(azimuthAng, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %22

                                                      // --- eleAng ---
                                                      .arg(QString("eleAng:").leftJustified(LABEL_WIDTH, ' ')) // %23
                                                      .arg(QString::number(eleAng, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %24

                                                      // --- radVelAbs ---
                                                      .arg(QString("radVelAbs:").leftJustified(LABEL_WIDTH, ' ')) // %25
                                                      .arg(QString::number(radVelAbs, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %26

                                                      // --- detValid (0位小数) ---
                                                      .arg(QString("detValid:").leftJustified(LABEL_WIDTH, ' ')) // %27
                                                      .arg(QString::number(detValid, 'f', 0).rightJustified(VALUE_WIDTH, ' ')) // %28

                                                      // --- rcsdB ---
                                                      .arg(QString("rcsdB:").leftJustified(LABEL_WIDTH, ' ')) // %29
                                                      .arg(QString::number(rcsdB, 'f', PRECISION).rightJustified(VALUE_WIDTH, ' ')) // %30

                                                      // --- rangebin ---
                                                      .arg(QString("Rangebin:").leftJustified(LABEL_WIDTH, ' ')) // %31
                                                      .arg(QString::number(rangebin).rightJustified(VALUE_WIDTH, ' ')) // %32

                                                        // --- velbin ---
                                                      .arg(QString("Velbin:").leftJustified(LABEL_WIDTH, ' ')) // %33
                                                      .arg(QString::number(velbin).rightJustified(VALUE_WIDTH, ' ')); // %34

                            QToolTip::showText(me->globalPos(), toolTipText, ui->qvtkWidget);
                            return false; // 允许 VTK 事件继续
                        }
                    }
                }




                // 如果鼠标移动但没有按键，且没有拾取到点，则隐藏提示
                if (me->buttons() == Qt::NoButton) {
                    QToolTip::hideText();
                }
            }
            // --- 🌟 悬停提示逻辑结束 🌟 ---


            if (me && me->buttons() != Qt::NoButton) {
                // MouseMove 需要当前按钮信息，但其 InvokeEvent 不依赖 button_key
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, 0, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
                // return false;
            }
            return false;
        }

        // ------------------------------------
        // C. 鼠标释放事件 (Release): 结束交互
        // ------------------------------------
        else if (event->type() == QEvent::MouseButtonRelease) {

            char button_key = '0'; // VTK 按钮代码

            if (me->button() == Qt::LeftButton) {
                button_key = '1';
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, button_key, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
                vtkCamera* camera = mainRenderer->GetActiveCamera();
                        if (camera) {
                            double pos[3], fp[3], vu[3];
                            camera->GetPosition(pos);
                            camera->GetFocalPoint(fp);
                            camera->GetViewUp(vu);

                            qDebug() << "Mouse Release Camera:";
                            qDebug() << " Position:" << pos[0] << pos[1] << pos[2];
                            qDebug() << " FocalPoint:" << fp[0] << fp[1] << fp[2];
                            qDebug() << " ViewUp:" << vu[0] << vu[1] << vu[2];
                            qDebug() << " Distance:" << camera->GetDistance();
                            qDebug() << " ViewAngle:" << camera->GetViewAngle();
                        }

                return false;
            } else if (me->button() == Qt::MiddleButton || (me->button() == Qt::LeftButton && me->modifiers() & Qt::ControlModifier)) {
                button_key = '2';
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, button_key, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent, nullptr);
                return false;
            } else if (me->button() == Qt::RightButton) {
                button_key = '3';
                interactor->SetEventInformation(me->x(), me->y(), ctrl, shift, button_key, 0, nullptr);
                interactor->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
                return false;
            }
        }

        // ------------------------------------
        // D. 滚轮事件 (Wheel): 缩放
        // ------------------------------------
        else if (event->type() == QEvent::Wheel) {
            QWheelEvent* we = static_cast<QWheelEvent*>(event);

            if (we->angleDelta().y() > 0) {
                interactor->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
            } else {
                interactor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
            }


            return false;
        }
    }

    // 4. 未处理的事件传递给基类
    if (obj == ui->qvtkWidget && event->type() == QEvent::Resize) {

        updateInfoPanelPosition();
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::doAreaPicknew(int x0, int y0, int x1, int y1)
{
    // =========================================================================
        // 🌟 关键修正：检查数据是否已加载 🌟
        // =========================================================================

        // 1. 获取当前帧索引，确保其有效
        int currentFrameIndex = ui->frameSlider->value();
        if (currentFrameIndex < 0 || currentFrameIndex >= multiFrames.size()) {
            qWarning() << "Selection Error: Invalid frame index. No data loaded or index out of range.";
            return;
        }

        // 2. 确保 satecloud 指向有效数据（如果 satecloud 不是全局指针）
        // 假设您在加载点云时将当前帧数据赋给了 satecloud
        if (!satecloud) { // 或者检查 satecloud 是否为空
            qWarning() << "Selection Error: Point cloud data (satecloud) is nullptr.";
            return;
        }

        // 3. 确保点云内有点（防止空点云崩溃）
        if (satecloud->points.empty()) { // 假设 satecloud 是 PCL 结构或具有 .points.empty() 方法
            qWarning() << "Selection Error: Current point cloud is empty.";
            return;
        }
    int minX = std::min(x0, x1);
    int maxX = std::max(x0, x1);
    int minY = std::min(y0, y1);
    int maxY = std::max(y0, y1);

    vtkRenderWindow* rw = ui->qvtkWidget->renderWindow();
    vtkRenderer* mainRenderer = rw->GetRenderers()->GetFirstRenderer();

    areaPicker->AreaPick(minX, minY, maxX, maxY, mainRenderer);
    vtkSmartPointer<vtkPlanes> frustum = areaPicker->GetFrustum();


    // ------------------- 显示框选区域 -------------------

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.001);

    // Pick 第一个点
    picker->Pick(x0, y0, 0, mainRenderer);
    double pt1[3];
    picker->GetPickPosition(pt1);

    // Pick 第二个点
    picker->Pick(x1, y1, 0, mainRenderer);
    double pt2[3];
    picker->GetPickPosition(pt2);

    // 如果两个点在同一位置或距离太近，提前返回或打印错误
    if (vtkMath::Distance2BetweenPoints(pt1, pt2) < 1e-6) {
        std::cerr << "Picked points too close or identical. Cannot form a valid plane." << std::endl;
        return;
    }

    // 固定 Z 坐标
    const double zPlane = (pt1[2] + pt2[2]) / 2.0;

    // 使用三个点定义平面，确保不共线
    // Origin: pt1
    // Point1: 与 pt1 X 不同
    // Point2: 与 pt1 Y 不同

    double origin[3]  = { pt1[0], pt1[1], zPlane };
    double point1[3]  = { pt2[0], pt1[1], zPlane };  // 同 Y

    double point2[3];
    if (std::abs(pt2[1] - pt1[1]) < 1e-6) {
        // 避免共线，加一点偏移
        point2[0] = pt1[0];
        point2[1] = pt1[1] + 1.0; // 人工加一点高度
        point2[2] = zPlane;
    } else {
        point2[0] = pt1[0];
        point2[1] = pt2[1];
        point2[2] = zPlane;
    }

    vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetOrigin(origin);
    planeSource->SetPoint1(point1);
    planeSource->SetPoint2(point2);
    planeSource->Update();



    // 创建mapper和actor
    vtkSmartPointer<vtkPolyDataMapper> mappers = vtkSmartPointer<vtkPolyDataMapper>::New();
    mappers->SetInputConnection(planeSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mappers);
    actor->GetProperty()->SetColor(0, 1, 0);  // 绿色
    actor->GetProperty()->SetOpacity(0.3);     // 半透明
    actor->GetProperty()->SetRepresentationToSurface();

    // 移除旧的actor（如果有）
    if (existingPlaneActor)
        mainRenderer->RemoveActor(existingPlaneActor);
    existingPlaneActor = actor;


    existingPlaneActor->SetPickable(false); // 🌟 关键：设置框选平面不可拾取

    mainRenderer->AddActor(actor);
    // ui->qvtkWidget->renderWindow()->Render();





    // 转换点云为 PolyData
    vtkSmartPointer<vtkPolyData> polyData = PointCloudLoader::convertmydataToVTKPolyData(satecloud);

    // 添加 PointId
    vtkSmartPointer<vtkIdFilter> idFilter = vtkSmartPointer<vtkIdFilter>::New();
    idFilter->SetInputData(polyData);
    idFilter->SetPointIds(true);
    idFilter->SetPointIdsArrayName("OriginalIds");
    idFilter->Update();

    // 用 Frustum 提取区域内的点
    vtkSmartPointer<vtkExtractGeometry> extractGeometry = vtkSmartPointer<vtkExtractGeometry>::New();
    extractGeometry->SetImplicitFunction(frustum);

    extractGeometry->SetInputData(idFilter->GetOutput());
    extractGeometry->Update();

    // 获取结果
    vtkSmartPointer<vtkUnstructuredGrid> unstructured = extractGeometry->GetOutput();

    vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometryFilter->SetInputData(unstructured);
    geometryFilter->Update();

    vtkSmartPointer<vtkPolyData> selectedData = geometryFilter->GetOutput();


    /*
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    for (vtkIdType i = 0; i < selectedData->GetNumberOfPoints(); ++i)
    {
        unsigned char green[3] = {0, 255, 0};
        colors->InsertNextTypedTuple(green);
    }


    selectedData->GetPointData()->SetScalars(colors);
*/
    // 可视化
    selectedRenderer->RemoveAllViewProps();

    vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    glyphFilter->SetInputData(selectedData);
    glyphFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyphFilter->GetOutputPort());
    //mapper->ScalarVisibilityOn();


    selectedActor = vtkSmartPointer<vtkActor>::New();
    selectedActor->SetMapper(mapper);
    selectedActor->GetProperty()->SetColor(1, 1, 0); // 黄色
    selectedActor->SetPickable(false);

    selectedRenderer->AddActor(selectedActor);
    selectedRenderer->ResetCamera();
    ui->qvtkWidget->renderWindow()->Render();

    // 显示属性
    ui->comboBoxSelectedPoints->clear();
    // =========================================================================
    // 🌟 核心修正区：累积/替换逻辑 和 多帧键控 🌟
    // =========================================================================

    // 1. 获取当前帧索引 和 playframecount 值

    // 🎯 请将 '1' 替换为获取 playframecount 值的代码，例如：ui->spinBoxPlayFrameCount->value();
    int currentPlayFrameCount = ui->playframecount->value();

    // 2. 替换逻辑：如果 playframecount == 1，则清空所有选中点 (替换模式)
    if (currentPlayFrameCount == 1 && !selectedPointKeys.isEmpty()) {
        QList<QPair<int, int>> keysToClear = selectedPointKeys.values();

        for (const auto& key : keysToClear) {
            // 安全移除，依赖 removeSelectionByKey 完成 VTK/ComboBox 清理
            if (selectedPointKeys.contains(key)) {
                selectedPointKeys.remove(key);
                removeSelectionByKey(key);
            }
        }
    }

    // 3. 遍历选中的点并调用新函数
    // 提示：ComboBox 清空逻辑在上面替换模式中已由 removeSelectionByKey 间接完成，
    // 或在累积模式中保持不变。这里不再需要 ui->comboBoxSelectedPoints->clear();

    vtkDataArray* idArray = selectedData->GetPointData()->GetArray("OriginalIds");
    vtkIdType numPoints = selectedData->GetNumberOfPoints();

    for (vtkIdType i = 0; i < numPoints; ++i) {
        vtkIdType originalId = idArray ? static_cast<vtkIdType>(idArray->GetComponent(i, 0)) : -1;

        if (originalId >= 0) {
            // 🎯 使用新的多帧键控函数

            addPointToSelectionnew(currentFrameIndex, static_cast<int>(originalId));
        }
    }

    // 4. 刷新渲染 (如果上面 VTK 渲染没有触发)
    // ui->qvtkWidget->renderWindow()->Render(); // 保持调用一次

    // 5. 更新 ComboBox 和信息面板
    if (ui->comboBoxSelectedPoints->count() > 0) {
        // 选择第一个点进行信息显示
        ui->comboBoxSelectedPoints->setCurrentIndex(0);

        // 🎯 手动读取 UserRole 数据并设置 label
        QVariant data = ui->comboBoxSelectedPoints->itemData(0, Qt::UserRole);
        if (data.isValid()) {
            ui->labelPointInfo->setText(data.toString());
        } else {
            ui->labelPointInfo->clear();
        }

        // 移除 on_selectedPointChanged(0); 的调用，因为 setCurrentIndex(0) 应该会触发 signal/slot
        // 如果没有连接，则上面的手动设置是必要的。
    } else {
        ui->labelPointInfo->clear();
    }
}

void MainWindow::doAreaPick(int x0, int y0, int x1, int y1)
{
    int minX = std::min(x0, x1);
    int maxX = std::max(x0, x1);
    int minY = std::min(y0, y1);
    int maxY = std::max(y0, y1);

    vtkRenderWindow* rw = ui->qvtkWidget->renderWindow();
    vtkRenderer* mainRenderer = rw->GetRenderers()->GetFirstRenderer();

    areaPicker->AreaPick(minX, minY, maxX, maxY, mainRenderer);
    vtkSmartPointer<vtkPlanes> frustum = areaPicker->GetFrustum();


    // ------------------- 显示框选区域 -------------------

    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.001);

    // Pick 第一个点
    picker->Pick(x0, y0, 0, mainRenderer);
    double pt1[3];
    picker->GetPickPosition(pt1);

    // Pick 第二个点
    picker->Pick(x1, y1, 0, mainRenderer);
    double pt2[3];
    picker->GetPickPosition(pt2);

    // 如果两个点在同一位置或距离太近，提前返回或打印错误
    if (vtkMath::Distance2BetweenPoints(pt1, pt2) < 1e-6) {
        std::cerr << "Picked points too close or identical. Cannot form a valid plane." << std::endl;
        return;
    }

    // 固定 Z 坐标
    const double zPlane = (pt1[2] + pt2[2]) / 2.0;

    // 使用三个点定义平面，确保不共线
    // Origin: pt1
    // Point1: 与 pt1 X 不同
    // Point2: 与 pt1 Y 不同

    double origin[3]  = { pt1[0], pt1[1], zPlane };
    double point1[3]  = { pt2[0], pt1[1], zPlane };  // 同 Y

    double point2[3];
    if (std::abs(pt2[1] - pt1[1]) < 1e-6) {
        // 避免共线，加一点偏移
        point2[0] = pt1[0];
        point2[1] = pt1[1] + 1.0; // 人工加一点高度
        point2[2] = zPlane;
    } else {
        point2[0] = pt1[0];
        point2[1] = pt2[1];
        point2[2] = zPlane;
    }

    vtkSmartPointer<vtkPlaneSource> planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetOrigin(origin);
    planeSource->SetPoint1(point1);
    planeSource->SetPoint2(point2);
    planeSource->Update();



    // 创建mapper和actor
    vtkSmartPointer<vtkPolyDataMapper> mappers = vtkSmartPointer<vtkPolyDataMapper>::New();
    mappers->SetInputConnection(planeSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mappers);
    actor->GetProperty()->SetColor(0, 1, 0);  // 绿色
    actor->GetProperty()->SetOpacity(0.3);     // 半透明
    actor->GetProperty()->SetRepresentationToSurface();

    // 移除旧的actor（如果有）
    if (existingPlaneActor)
        mainRenderer->RemoveActor(existingPlaneActor);
    existingPlaneActor = actor;


    existingPlaneActor->SetPickable(false); // 🌟 关键：设置框选平面不可拾取

    mainRenderer->AddActor(actor);
    // ui->qvtkWidget->renderWindow()->Render();





    // 转换点云为 PolyData
    vtkSmartPointer<vtkPolyData> polyData = PointCloudLoader::convertmydataToVTKPolyData(satecloud);

    // 添加 PointId
    vtkSmartPointer<vtkIdFilter> idFilter = vtkSmartPointer<vtkIdFilter>::New();
    idFilter->SetInputData(polyData);
    idFilter->SetPointIds(true);
    idFilter->SetPointIdsArrayName("OriginalIds");
    idFilter->Update();

    // 用 Frustum 提取区域内的点
    vtkSmartPointer<vtkExtractGeometry> extractGeometry = vtkSmartPointer<vtkExtractGeometry>::New();
    extractGeometry->SetImplicitFunction(frustum);

    extractGeometry->SetInputData(idFilter->GetOutput());
    extractGeometry->Update();

    // 获取结果
    vtkSmartPointer<vtkUnstructuredGrid> unstructured = extractGeometry->GetOutput();

    vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
    geometryFilter->SetInputData(unstructured);
    geometryFilter->Update();

    vtkSmartPointer<vtkPolyData> selectedData = geometryFilter->GetOutput();


    /*
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");
    for (vtkIdType i = 0; i < selectedData->GetNumberOfPoints(); ++i)
    {
        unsigned char green[3] = {0, 255, 0};
        colors->InsertNextTypedTuple(green);
    }


    selectedData->GetPointData()->SetScalars(colors);
*/
    // 可视化
    selectedRenderer->RemoveAllViewProps();

    vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    glyphFilter->SetInputData(selectedData);
    glyphFilter->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(glyphFilter->GetOutputPort());
    //mapper->ScalarVisibilityOn();


    selectedActor = vtkSmartPointer<vtkActor>::New();
    selectedActor->SetMapper(mapper);
    selectedActor->GetProperty()->SetColor(1, 1, 0); // 黄色
    selectedActor->SetPickable(false);

    selectedRenderer->AddActor(selectedActor);
    selectedRenderer->ResetCamera();
    ui->qvtkWidget->renderWindow()->Render();

    // 显示属性
    ui->comboBoxSelectedPoints->clear();

    vtkPoints* points = selectedData->GetPoints();
    vtkDataArray* idArray = selectedData->GetPointData()->GetArray("OriginalIds");
    vtkIdType numPoints = selectedData->GetNumberOfPoints();

    for (vtkIdType i = 0; i < numPoints; ++i) {
        vtkIdType originalId = idArray ? static_cast<vtkIdType>(idArray->GetComponent(i, 0)) : -1;
        if (originalId >= 0)
            addPointToSelection(originalId);
    }


    if (ui->comboBoxSelectedPoints->count() > 0) {
        ui->comboBoxSelectedPoints->setCurrentIndex(0);
        on_selectedPointChanged(0);
    } else {
        ui->labelPointInfo->clear();
    }

}



void MainWindow::on_selectedPointChanged(int index)
{
    if (index < 0) {
        ui->labelPointInfo->clear();
        return;
    }

    QString fullInfo = ui->comboBoxSelectedPoints->itemData(index, Qt::UserRole).toString();
    ui->labelPointInfo->setText(fullInfo);
}


template <typename PointT>
QString MainWindow::getPointInfonew(const PointT& pt, const QPair<int, int>& selectionKey) {

    int frameIndex = selectionKey.first;
    int originalPointId = selectionKey.second;

    // 1. ID 信息 (作为纯文本/段落，不包含在表格内)
    QString idInfo = QString(
                         "<p style='color: #DDDDDD; font-size: 14pt; margin-bottom: 10px;'>"
                         "<b>Frame ID: %1</b> &nbsp;&nbsp;&nbsp;&nbsp; <b>Original ID: %2</b>"
                         "</p>"
                         "<hr style='border: 0.5px solid #444444;'>"
                         )
                         .arg(QString::number(frameIndex))
                         .arg(QString::number(originalPointId));


    // 2. 属性表格 (只包含属性)
    QString tableInfo = QString(
                            "<table style='color: #DDDDDD; width: 100%; text-align: center;' border='0' cellspacing='5' cellpadding='5'>"
                            // 原始属性表格头
                            "<tr>"
                            "<th>X</th><th>Y</th><th>Z</th><th>Range</th><th>DopplerSpeed</th>"
                            "<th>PowerdB</th><th>SNRdB</th><th>Q_azi</th><th>Q_ele</th>"
                            "<th>AzimuthAng</th><th>EleAng</th><th>radVelAbs</th><th>detValid</th><th>RcsdB</th>"
                            "</tr>"
                            // 原始属性表格内容
                            "<tr>"
                            "<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>"
                            "<td>%6</td><td>%7</td><td>%8</td><td>%9</td>"
                            "<td>%10</td><td>%11</td><td>%12</td><td>%13</td><td>%14</td>"
                            "</tr>"
                            "</table>")

                            // 注意：占位符从 %1 开始，因为 ID 信息已经移到 tableInfo 外面
                            .arg(QString::number(pt.x,             'f', 3)) // %1 X
                            .arg(QString::number(pt.y,             'f', 3)) // %2 Y
                            .arg(QString::number(pt.z,             'f', 3)) // %3 Z
                            .arg(QString::number(pt.range,         'f', 3)) // %4 Range
                            .arg(QString::number(pt.dopplerSpeed,  'f', 5)) // %5 DopplerSpeed
                            .arg(QString::number(pt.powerdB,       'f', 2)) // %6 PowerdB
                            .arg(QString::number(pt.SNRdB,         'f', 2)) // %7 SNRdB
                            .arg(QString::number(pt.Q_azi,         'f', 5)) // %8 Q_azi
                            .arg(QString::number(pt.Q_ele,         'f', 5)) // %9 Q_ele
                            .arg(QString::number(pt.azimuthAng,    'f', 3)) // %10 AzimuthAng
                            .arg(QString::number(pt.eleAng,        'f', 3)) // %11 EleAng
                            .arg(QString::number(pt.radVelAbs,     'f', 3)) // %12 radVelAbs
                            .arg(QString::number(pt.detValid,      'f', 0)) // %13 detValid
                            .arg(QString::number(pt.rcsdB,         'f', 1)); // %14 RcsdB

    return idInfo + tableInfo;
}

template <typename PointT>
QString MainWindow::getPointInfo(const PointT& pt, int id) {

    /*QString info = QString(
                           "X:            %2\n"
                           "Y:            %3\n"
                           "Z:            %4\n"
                           "Range:        %5     DopplerSpeed: %6\n"
                           "PowerdB:      %7     SNRdB:        %8\n"
                           "Q_azi:        %9     Q_ele:        %10\n"
                           "AzimuthAng:   %11    EleAng:       %12\n"
                           "radVelAbs:    %13\n  RcsdB:        %14")
                                                     // ID：宽度 4
                       .arg(pt.x, 10, 'f', 3)                    // 宽度 10，保留 3 位小数
                       .arg(pt.y, 10, 'f', 3)
                       .arg(pt.z, 10, 'f', 3)
                       .arg(pt.range,        10, 'f', 3)
                       .arg(pt.dopplerSpeed, 10, 'f', 5)
                       .arg(pt.powerdB,      10, 'f', 2)
                       .arg(pt.SNRdB,        10, 'f', 2)
                       .arg(pt.Q_azi,        10, 'f', 5)
                       .arg(pt.Q_ele,        10, 'f', 5)
                       .arg(pt.azimuthAng,   10, 'f', 3)
                       .arg(pt.eleAng,       10, 'f', 3)
                       .arg(pt.radVelAbs,    10, 'f', 3)
                       .arg(pt.rcsdB,        10, 'f', 1);*/

    QString info = QString(
                       "<table border='0' cellspacing='10'>"
                       "<tr>"
                       "<th>X</th><th>Y</th><th>Z</th><th>Range</th><th>DopplerSpeed</th>"
                       "<th>PowerdB</th><th>SNRdB</th><th>Q_azi</th><th>Q_ele</th>"
                       "<th>AzimuthAng</th><th>EleAng</th><th>radVelAbs</th><th>detValid</th><th>RcsdB</th>"
                       "</tr>"
                       "<tr>"
                       "<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>"
                       "<td>%6</td><td>%7</td><td>%8</td><td>%9</td>"
                       "<td>%10</td><td>%11</td><td>%12</td><td>%13</td><td>%14</td>"
                       "</tr>"
                       "</table>")

                       .arg(pt.x,           0, 'f', 3)
                       .arg(pt.y,           0, 'f', 3)
                       .arg(pt.z,           0, 'f', 3)
                       .arg(pt.range,       0, 'f', 3)
                       .arg(pt.dopplerSpeed,0, 'f', 5)
                       .arg(pt.powerdB,     0, 'f', 2)
                       .arg(pt.SNRdB,       0, 'f', 2)
                       .arg(pt.Q_azi,       0, 'f', 5)
                       .arg(pt.Q_ele,       0, 'f', 5)
                       .arg(pt.azimuthAng,  0, 'f', 3)
                       .arg(pt.eleAng,      0, 'f', 3)
                       .arg(pt.radVelAbs,   0, 'f', 3)
                       .arg(pt.detValid,    0, 'f', 0)
                       .arg(pt.rcsdB,       0, 'f', 1);

    // 特殊字段，仅当是带属性点类型时添加


    return info;
}

template <typename CloudT>
void MainWindow::handlePickedPoint(const CloudT& cloud, int pickedPointId) {
    // addPointToSelection(pickedPointId);
    // 1. 获取 Picker 命中的 Actor (即 pointCloudActor)
    // pointPicker 是 MainWindow 的成员，可以直接访问
    vtkActor* hitActor = pointPicker->GetActor();
    if (!hitActor) return;


    // 2. 从 Mapper 获取 PolyData (PolyData 包含我们存储的 PointData)
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(hitActor->GetMapper()->GetInputDataObject(0, 0));
    if (!polyData) return;

    // 3. 查找我们存储的原始 ID 数组
    vtkIntArray* originalIdArray = vtkIntArray::SafeDownCast(
        polyData->GetPointData()->GetArray("OriginalID") // 确保名称 "OriginalID" 匹配
        );

    if (originalIdArray && pickedPointId >= 0 && pickedPointId < originalIdArray->GetNumberOfTuples()) {

        // 4. 提取原始 ID：这是 satecloud 中的正确索引！
        int originalPointId = originalIdArray->GetValue(pickedPointId);

        // 5. 将正确的原始 ID 添加到选中列表
        addPointToSelection(originalPointId);

    } else {
        // 如果没有筛选，或者 ID 数组丢失，则回退到 Picker ID
        // 注意：如果筛选开启，这里可能会选中错误的点

        addPointToSelection(pickedPointId);
    }
}

void MainWindow::on_readRadar4T4Bin_clicked()
{
    multiFrames.clear();
    frames8x8.clear();
        clearHighlightedPoints();
        timer->stop();
    if (satecloud) {
            // 检查 satecloud 是否有效，防止空指针解引用
            satecloud->clear(); // 清空点云数据，但对象仍然存在

            // 如果想要彻底释放这块内存并清空指针，使用 reset()：
            satecloud.reset(); // satecloud 现在是 nullptr，并且内存被释放（如果这是最后一个引用）
    }

    // --- 2. 🌟 关键修正：清理 VTK 渲染 Actor 🌟
        if (pointCloudActor) {
            // 从渲染器中移除旧的点云 Actor
            mainRenderer->RemoveActor(pointCloudActor);
            // 将 Actor 指针设为 nullptr，以便 displayPointCloud() 知道需要重新创建 Actor
            pointCloudActor = nullptr;
        }
    isFirstFrame = true;
    if(ui->signal4_4->isChecked()){
    QString filename = QFileDialog::getOpenFileName(this, "ReadRadar4*4ADCBIN", "", "Bin Files (*.bin)");
    if (filename.isEmpty()) return;

    QString savePath = QFileDialog::getSaveFileName(this , "Save Pointdata to CSV", "", "CSV 文件 (*.csv)");
    if (savePath.isEmpty())
        return;


    GetRadar4T4DatafromBIN(filename,savePath);


    QString fileCSVtoPCDName = QFileDialog::getSaveFileName(this, "保存点云文件", "", "PCD 文件 (*.pcd)");
    if (fileCSVtoPCDName.isEmpty())
        return;

    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> satedata;
    satedata = customdata.SateliteRadarData(savePath.toStdString());
    // PCL保存PCD文件（ASCII格式）
    if (pcl::io::savePCDFileASCII(fileCSVtoPCDName.toStdString(), *satedata) == 0) {
        QMessageBox::information(this, "保存成功", "点云已保存至:\n" + fileCSVtoPCDName);

        if (loadedFilesDock) {
                        loadedFilesDock->updateADCBin(QFileInfo(filename).fileName());
                    }

    } else {
        QMessageBox::critical(this, "保存失败", "无法保存点云文件！");
    }}
    else if(ui->signal8_8->isChecked()){
        QString filename = QFileDialog::getOpenFileName(this, "ReadRadar8*8ADCBIN", "", "Bin Files (*.bin)");
        if (filename.isEmpty()) return;
        // TransferOneFrame8_8RadarADC(filename);
        rdMapPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        // QString filenamed="C:/Users/JPN1WX/Desktop/QT/PointCloudUI/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/output8x8.bin";
        std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> radar8x8pointdata = setupRangeDopplerPlotnew(rdMapPlot,filename);
        if (radar8x8pointdata && !radar8x8pointdata->empty()) {

            const int currentFrameIndex = 0;
            for (size_t i = 0; i < radar8x8pointdata->points.size(); ++i) {
                            auto& pt = radar8x8pointdata->points[i];
                            pt.frameIndex = currentFrameIndex;
                            pt.originalIndex = static_cast<int>(i); // 使用点在数组中的索引
                        }
            multiFrames.push_back(radar8x8pointdata);
            // 3. 更新全局点云指针 (参考 on_loadButton_clicked 逻辑)
            satecloud = radar8x8pointdata;

            calculateCloudMetrics(satecloud);
            pointandtimedata8x8 tempFrame;
            tempFrame.Radar8T8Point.cloud = radar8x8pointdata;
            std::vector<pointandtimedata8x8> tempFrames = {tempFrame};
            pcMinMax = calculate8T8MinMax(tempFrames); // 重新计算所有帧的 Min/Max
                    initSlidersFromPointCloud(pcMinMax.get()); // 根据计算出的 Min/Max 设置滑块范围
            // 4. 对新加载的点云进行着色
            // 确保 colorizer 能够访问到当前颜色模式所需的最大/最小值。
            // colorizer.colorizePointCloud(satecloud, currentColorMode);

            // 5. 设置标志并调用显示函数
            isFirstFrame = true;  // 重置视角
            displayPointCloud();

            // 可选：显示成功消息
            QMessageBox::information(this, "数据加载成功",
                                     QString("已加载 %1 个 8x8 点云数据。")
                                         .arg(satecloud->size()));
            if (loadedFilesDock) {
                           loadedFilesDock->updateADCBin(QFileInfo(filename).fileName());
                       }

        } else {
            QMessageBox::warning(this, "加载失败", "未生成或获取到 8x8 点云数据！");
        }
    }
}

void MainWindow::addPointIdToComboBoxnew(const QPair<int, int>& selectionKey)
{
    int frameIndex = selectionKey.first;
    int originalPointId = selectionKey.second;

    // 1. 创建显示给用户的文本 (包含 Frame ID 和 Original ID)
    QString comboText = QString("[F:%1] ID:%2").arg(frameIndex).arg(originalPointId);

    // 2. 遍历已有项，检查是否已存在 (防重复添加)
    bool exists = false;
    for (int i = 0; i < ui->comboBoxSelectedPoints->count(); ++i) {

        // 🌟 关键修正：从存储的 User Data (Qt::UserRole + 1) 中读取 Key 🌟
        QVariant data = ui->comboBoxSelectedPoints->itemData(i, Qt::UserRole + 1);

        if (data.isValid() && data.canConvert<QPair<int, int>>()) {
            QPair<int, int> itemKey = data.value<QPair<int, int>>();
            if (itemKey == selectionKey) {
                exists = true;
                break;
            }
        }
        // 如果旧的逻辑 (只比较 itemText) 更简单，也可以保留:
        // if (ui->comboBoxSelectedPoints->itemText(i) == comboText) { exists = true; break; }
    }

    // 3. 如果不存在，则添加新项
    if (!exists) {
        int newIndex = ui->comboBoxSelectedPoints->count();
        ui->comboBoxSelectedPoints->addItem(comboText);

        // 🌟 关键：将 selectionKey (QPair<int, int>) 存储为 User Data 🌟
        // 必须使用 QVariant::fromValue 转换
        ui->comboBoxSelectedPoints->setItemData(
            newIndex,
            QVariant::fromValue(selectionKey),
            Qt::UserRole + 1 // 用于存储唯一的键，以便在 removeSelectionByKey 中查找
            );

        // 备注: 详细信息 (getPointInfo 的结果) 应该在 addPointToSelection 中存储到 Qt::UserRole。
    }
}

void MainWindow::addPointIdToComboBox(int pickedPointId) {
    QString idStr = "ID:" + QString::number(pickedPointId);

    // 遍历已有项，看看有没有
    bool exists = false;
    for (int i = 0; i < ui->comboBoxSelectedPoints->count(); ++i) {
        if (ui->comboBoxSelectedPoints->itemText(i) == idStr) {
            exists = true;
            break;
        }


    }

    if (!exists) {
        ui->comboBoxSelectedPoints->addItem(idStr);


    }
}


void MainWindow::removePointFromHighlighted(int pickedPointId) {

    // 🌟 关键检查：确保 Mapper 存在且有效 🌟
    if (!highlightedMapper) {
        // 如果 Mapper 不存在，说明高亮 Actor 已经被移除或从未创建，无需更新数据流
        return;
    }

    vtkNew<vtkPoints> newPoints;

    // 注意：selectedPointIds 应该在调用此函数之前已经被修改 (移除了 pickedPointId)
    for (int id : selectedPointIds) {
        // 确保 ID 有效，尽管理论上不需要，但安全起见
        if (id >= 0 && id < static_cast<int>(satecloud->points.size())) {
            const auto& pt = satecloud->points[id];
            newPoints->InsertNextPoint(pt.x, pt.y, pt.z);
        } else {
            qWarning() << "Warning: Invalid ID found in selectedPointIds:" << id;
        }
    }

    // 检查是否有剩余点，如果没有，清理 Actor
    if (newPoints->GetNumberOfPoints() == 0) {
        if (highlightedActor) {
            mainRenderer->RemoveActor(highlightedActor);
            highlightedActor = nullptr;
            // 确保其他相关对象也被清理或标记为空，避免悬空指针
            // 例如：highlightedMapper = nullptr; highlightedPolyData = nullptr;
            // 如果它们是 vtkSmartPointer 成员，这一步不是必须的，但确保逻辑一致性
        }
        return;
    }

    // 更新数据
    highlightedPoints->DeepCopy(newPoints);
    highlightedPoints->Modified();

    // 修复：highlightedPolyData 应该在初始化时就设置了 highlightedPoints，这里无需重复设置
    // highlightedPolyData->SetPoints(highlightedPoints);
    // 如果您在初始化时已设置，这里可以注释掉，因为 highlightedPoints->Modified() 已经足够通知 PolyData更新

    // 获取并更新 Glyph Filter
    auto* glyphFilter = vtkVertexGlyphFilter::SafeDownCast(
        highlightedMapper->GetInputConnection(0, 0)->GetProducer()
        );

    if (glyphFilter) {
        glyphFilter->Update();
    } else {
        qWarning() << "Error: Could not retrieve vtkVertexGlyphFilter for highlighting.";
    }

    // 不需要 Render，因为 addPointToSelection 的外部会调用 Render
}

void MainWindow::removePointFromComboBox(int pickedPointId) {
    int removeIndex = -1;
    for (int i = 0; i < ui->comboBoxSelectedPoints->count(); ++i) {
        if (ui->comboBoxSelectedPoints->itemText(i).contains(QString::number(pickedPointId))) {
            removeIndex = i;
            break;
        }
    }

    if (removeIndex != -1) {
        ui->comboBoxSelectedPoints->removeItem(removeIndex);

        int count = ui->comboBoxSelectedPoints->count();
        if (count > 0) {
            // 删除后选择删除条目索引或最后一个条目
            int newIndex = (removeIndex < count) ? removeIndex : count - 1;
            ui->comboBoxSelectedPoints->setCurrentIndex(newIndex);

            // 手动更新 label 显示新选中点的信息
            QVariant data = ui->comboBoxSelectedPoints->itemData(newIndex, Qt::UserRole);
            if (data.isValid()) {
                ui->labelPointInfo->setText(data.toString());
            } else {
                ui->labelPointInfo->clear();
            }
        } else {
            ui->labelPointInfo->clear();
        }
    }
}

void MainWindow::on_LoadVideo_clicked()
{

    QString filename = QFileDialog::getOpenFileName(
        this,
        "选择视频文件",
        "",
        "Video Files (*.avi *.mp4 *.mkv *.mov *.flv);;All Files (*.*)"
        );
    if (filename.isEmpty())
        return;

    m_player->setMedia(QUrl::fromLocalFile(filename)); // 你的 AVI 路径

    m_player->setPosition(0);
    m_player->play();
    m_player->pause();

    ui->DisplayVideo->setChecked(true);

    // 更新 slider 和 label（可选，如果 duration 还没触发 signal，可以手动更新）
    ui->horizontalSlider->setValue(0);
    ui->labelPosition->setText(formatTime(0));

    if (loadedFilesDock) {
            loadedFilesDock->updateVideo(QFileInfo(filename).fileName()); // 只显示文件名
            // 如果想显示完整路径：
            // loadedFilesDock->updateVideo(filename);
        }

    // 弹出提示
    QMessageBox::information(this, "加载成功",
                             QString("成功加载视频文件：%1").arg(filename));
}


void MainWindow::addPointToSelection(int pointId)
{
    if (pointId < 0 || pointId >= static_cast<int>(satecloud->points.size()))
        return;

    const auto& pt = satecloud->points[pointId];

    // ==========================================================
    // 🌟 关键修正：确保高亮 Actor 和管线存在 🌟
    // ==========================================================
    if (!highlightedActor) {
        // 1. 创建数据对象
        highlightedPoints = vtkSmartPointer<vtkPoints>::New();
        highlightedPolyData = vtkSmartPointer<vtkPolyData>::New();
        highlightedPolyData->SetPoints(highlightedPoints);

        // 2. 创建并连接 Glyph Filter
        vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
        glyphFilter->SetInputData(highlightedPolyData);

        // 3. 创建 Mapper
        highlightedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        highlightedMapper->SetInputConnection(glyphFilter->GetOutputPort());

        // 4. 创建 Actor，设置属性
        highlightedActor = vtkSmartPointer<vtkActor>::New();
        highlightedActor->SetMapper(highlightedMapper);
        highlightedActor->GetProperty()->SetColor(1.0, 0.0, 0.0); // 红色高亮
        highlightedActor->GetProperty()->SetPointSize(10.0);

        // 5. 将 Actor 添加到渲染器
        mainRenderer->AddActor(highlightedActor);
    }
    // ==========================================================
    // ----------------------------------------------------------


    if (selectedPointIds.contains(pointId)) {
        // 已选中 → 取消 (省略了这部分的代码，假设 removePointFromHighlighted 正确)
        selectedPointIds.remove(pointId);
        removePointFromHighlighted(pointId);
        removePointFromComboBox(pointId);

        // 修复：如果点集清空，可能需要移除 Actor 避免显示空集 (可选)
        if (selectedPointIds.isEmpty()) {
            mainRenderer->RemoveActor(highlightedActor);
        }

        ui->qvtkWidget->renderWindow()->Render();
        ui->labelPointInfo->setText("已取消点: " + QString::number(pointId));
        return;
    }

    // 新增选中
    selectedPointIds.insert(pointId);

    // 修复：当 Actor 存在时，只需要更新 Points 并更新管线
    highlightedPoints->InsertNextPoint(pt.x, pt.y, pt.z);
    highlightedPoints->Modified();

    // 修复：直接获取 Mapper 的输入，然后更新管线
    vtkVertexGlyphFilter* glyphFilter = vtkVertexGlyphFilter::SafeDownCast(
        highlightedMapper->GetInputConnection(0, 0)->GetProducer());

    if (glyphFilter) {
        glyphFilter->Update();
    } else {
        qWarning() << "Error: Glyph filter not found in highlightedMapper's pipeline.";
    }


    ui->qvtkWidget->renderWindow()->Render();

    // ... (后续的 ComboBox 和 Info 逻辑) ...
    addPointIdToComboBox(pointId);

    int comboIndex = ui->comboBoxSelectedPoints->count() - 1;

    QString info = getPointInfo(pt, pointId);

    ui->comboBoxSelectedPoints->setItemData(comboIndex, info, Qt::UserRole);


    // ---- 手动刷新 label，即使是已选中点也能显示 ----

    for (int i = 0; i < ui->comboBoxSelectedPoints->count(); ++i) {

        if (ui->comboBoxSelectedPoints->itemText(i).contains(QString::number(pointId))) {

            ui->comboBoxSelectedPoints->setCurrentIndex(i);

            QVariant data = ui->comboBoxSelectedPoints->itemData(i, Qt::UserRole);

            ui->labelPointInfo->setText(data.isValid() ? data.toString() : "");

            break;

        }

    }
}

void MainWindow::removeSelectionByKey(const QPair<int, int>& selectionKey) {

    // --- 第 1 部分：更新高亮显示的 VTK 数据 (合并 removePointFromHighlighted 逻辑) ---

    if (!selectedPointKeys.contains(selectionKey)) {
            // 如果键不存在，可能是一个异常调用，但我们可以安全退出
            qWarning() << "Attempted to remove non-existent key from selectedPointKeys.";
            // 尽管如此，我们还是执行 VTK 重建，以防 VTK 状态不一致
        } else {
            selectedPointKeys.remove(selectionKey); // 🎯 关键：在此处移除！
        }

    // 🌟 关键检查：确保 Mapper 存在且有效 🌟
    if (!highlightedMapper) {
        return;
    }

    vtkNew<vtkPoints> newPoints;

    // 遍历剩余的选中的点 (selectedPointKeys 应该在调用此函数之前已移除 selectionKey)
    for (const QPair<int, int>& key : selectedPointKeys) {
        int frameIndex = key.first;
        int originalPointId = key.second;

        // 查找正确的点云帧
        if (frameIndex >= 0 && frameIndex < multiFrames.size()) {
            const auto& ptCloud = multiFrames[frameIndex];

            // 确保 ID 有效
            if (originalPointId >= 0 && originalPointId < static_cast<int>(ptCloud->points.size())) {
                const auto& pt = ptCloud->points[originalPointId];
                newPoints->InsertNextPoint(pt.x, pt.y, pt.z);
            } else {
                qWarning() << "Warning: Invalid OriginalID in key:" << originalPointId;
            }
        } else {
            qWarning() << "Warning: Invalid FrameID in key:" << frameIndex;
        }
    }

    // 检查是否有剩余点，如果没有，清理 Actor
    if (newPoints->GetNumberOfPoints() == 0) {
        if (highlightedActor) {
            mainRenderer->RemoveActor(highlightedActor);
            highlightedActor = nullptr;
            // 清理其他 VTK 智能指针 (可选，但在 nullptr 检查时有帮助)
            highlightedPoints = nullptr;
            highlightedPolyData = nullptr;
        }
    } else {
        // 更新数据
        highlightedPoints->DeepCopy(newPoints);
        highlightedPoints->Modified();

        // 获取并更新 Glyph Filter
        // auto* glyphFilter = vtkVertexGlyphFilter::SafeDownCast(
        //     highlightedMapper->GetInputConnection(0, 0)->GetProducer()
        //     );
        // if (glyphFilter) {
        //     glyphFilter->Update();
        // } else {
        //     qWarning() << "Error: Could not retrieve vtkVertexGlyphFilter for highlighting.";
        // }
    }

    // --- 第 2 部分：更新 ComboBox (合并 removePointFromComboBox 逻辑) ---

    int removeIndex = -1;
    for (int i = 0; i < ui->comboBoxSelectedPoints->count(); ++i) {
        // 🌟 关键修正：从 ComboBox 的 UserRole 中读取 Key 进行比较 🌟
        QVariant data = ui->comboBoxSelectedPoints->itemData(i, Qt::UserRole + 1); // 假设 Key 存储在这里
        if (data.isValid() && data.canConvert<QPair<int, int>>()) {
            QPair<int, int> itemKey = data.value<QPair<int, int>>();
            if (itemKey == selectionKey) {
                removeIndex = i;
                break;
            }
        }
    }

    if (removeIndex != -1) {
        ui->comboBoxSelectedPoints->removeItem(removeIndex);

        int count = ui->comboBoxSelectedPoints->count();
        if (count > 0) {
            // 删除后选择删除条目索引或最后一个条目
            int newIndex = (removeIndex < count) ? removeIndex : count - 1;
            ui->comboBoxSelectedPoints->setCurrentIndex(newIndex);

            // 手动更新 label 显示新选中点的信息
            QVariant data = ui->comboBoxSelectedPoints->itemData(newIndex, Qt::UserRole);
            if (data.isValid()) {
                ui->labelPointInfo->setText(data.toString());
            } else {
                ui->labelPointInfo->clear();
            }
        } else {
            ui->labelPointInfo->clear();
        }
    }
}

void MainWindow::addPointToSelectionnew(int frameIndex, int originalPointId)
{
    // 将 (FrameID, OriginalID) 组合成一个唯一的键
    QPair<int, int> selectionKey = qMakePair(frameIndex, originalPointId);

    // ==========================================================
    // 1. 查找正确的点云帧并验证索引
    // ==========================================================
    if (frameIndex < 0 || frameIndex >= multiFrames.size()) {
        qWarning() << "Error: Invalid frameIndex:" << frameIndex;
        return;
    }

    const auto& ptCloud = multiFrames[frameIndex];
    if (originalPointId < 0 || originalPointId >= static_cast<int>(ptCloud->points.size())) {
        qWarning() << "Error: Invalid originalPointId:" << originalPointId;
        return;
    }

    // 从正确的帧中获取点数据
    const auto& pt = ptCloud->points[originalPointId];

    // ==========================================================
    // 2. 检查并初始化/重建高亮 Actor (修复崩溃)
    // ==========================================================
    // 检查 highlightedPoints 和 highlightedActor 是否存在。如果其中一个不存在，必须重建整个管道。
    if (!highlightedActor || !highlightedPoints || !highlightedMapper) {

        // 确保所有 VTK 智能指针都被重新实例化（如果使用 vtkSmartPointer，这里是 New()）
        // 假设您在 MainWindow::MainWindow() 中只声明了它们，但第一次使用时才实例化。
        // 或者它们在 removeSelectionByKey 中被设为 nullptr 后，需要重新实例化。

        highlightedPoints = vtkSmartPointer<vtkPoints>::New();
        highlightedPolyData = vtkSmartPointer<vtkPolyData>::New();
        highlightedPolyData->SetPoints(highlightedPoints);

        vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
        glyphFilter->SetInputData(highlightedPolyData);

        highlightedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        highlightedMapper->SetInputConnection(glyphFilter->GetOutputPort());

        highlightedActor = vtkSmartPointer<vtkActor>::New();
        highlightedActor->SetMapper(highlightedMapper);

        // 设置 Actor 属性 (例如颜色和大小)
        highlightedActor->GetProperty()->SetColor(1.0, 0.0, 0.0); // 黄色
        highlightedActor->GetProperty()->SetPointSize(10.0); // 增大点的大小

        mainRenderer->AddActor(highlightedActor);
    }
    // ==========================================================
    // 3. 检查并处理取消选中 (使用 Key)
    // ==========================================================
    if (selectedPointKeys.contains(selectionKey)) {
        // 已选中 → 取消
        selectedPointKeys.remove(selectionKey);

        // 🌟 修正：移除高亮点和组合框项目需要新的逻辑 🌟
        removeSelectionByKey(selectionKey); // 您需要实现这个函数来移除 PolyData 和 ComboBox 中的点

        // if (selectedPointKeys.isEmpty()) {
        //     mainRenderer->RemoveActor(highlightedActor);
        // }

        ui->qvtkWidget->renderWindow()->Render();
        ui->labelPointInfo->setText(
            QString("已取消点: [Frame:%1, Index:%2]").arg(frameIndex).arg(originalPointId)
            );
        return;
    }

    // ==========================================================
    // 4. 新增选中 (使用 Key)
    // ==========================================================
    selectedPointKeys.insert(selectionKey);

    // 修复：添加点坐标到高亮 PolyData
    highlightedPoints->InsertNextPoint(pt.x, pt.y, pt.z);
    highlightedPoints->Modified();

    // 刷新管线 (代码不变)
    vtkVertexGlyphFilter* glyphFilter = vtkVertexGlyphFilter::SafeDownCast(
        highlightedMapper->GetInputConnection(0, 0)->GetProducer());
    if (glyphFilter) {
        glyphFilter->Update();
    } else {
        qWarning() << "Error: Glyph filter not found in highlightedMapper's pipeline.";
    }

    ui->qvtkWidget->renderWindow()->Render();

    // ==========================================================
    // 5. 更新 ComboBox 和信息面板
    // ==========================================================
    // 🌟 修正：ComboBox 显示 FrameID 和 OriginalID 🌟
    QString comboText = QString("[F:%1] ID:%2").arg(frameIndex).arg(originalPointId);

    // 假设您修改了 addPointIdToComboBox 接受 selectionKey
    addPointIdToComboBoxnew(selectionKey);

    int comboIndex = ui->comboBoxSelectedPoints->count() - 1;

    // 🌟 修正：getPointInfo 必须使用 pt 本身而不是 satecloud 索引 🌟
    // 并且我们给 ComboBox 存储 selectionKey 作为数据
    QString info = getPointInfonew(pt, selectionKey);

    ui->comboBoxSelectedPoints->setItemData(comboIndex, info, Qt::UserRole);
    ui->comboBoxSelectedPoints->setItemData(comboIndex, QVariant::fromValue(selectionKey), Qt::UserRole + 1); // 存储 Key

    // ---- 自动刷新 label ----
    // ... (手动刷新 label 逻辑不变，但需要搜索和显示基于新的 comboText)
    // ... (如果 combo box 足够复杂，可以直接设置当前 index 并读取 UserRole)

    ui->comboBoxSelectedPoints->setCurrentIndex(comboIndex);
    ui->labelPointInfo->setText(info);
}
void MainWindow::on_TransferMDF4toPointdata_clicked()
{
    if(ui->signal4_4->isChecked()){
        frames8x8.clear();
        frameBuffer.clear();                // 清除所有积累的旧帧！
        lastAccumulatedCloud.reset();       // 清除积累追踪器
        if (CarData.empty()) {
            QMessageBox::warning(this, "错误", "CarData is empty！,Please loading CarData First");
            return;
        }

        MdfExtractDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            QString input = dlg.inputFile();
            QString output = dlg.outputFile();

            if (input.isEmpty() || output.isEmpty()) {
                QMessageBox::warning(this, "错误", "请输入有效的输入/输出文件路径！");
                return;
            }



            if (dlg.isAllData()) {
                // 全部转换，忽略时间
                // 传入cardata std::vector<TimestampedCarData> CarData
                //GetRadar4T4Datafrommf4(input, output, 0.0, 0.0);
                GetRadar4T4Datafrommf4(input, output, 0.0, 0.0,CarData,CarSRSYawData);
                isRadar8x8Mode = false;
            } else {
                double start = dlg.startTime();
                double end   = dlg.endTime();

                if (end <= start) {
                    QMessageBox::warning(this, "错误", "结束时间必须大于开始时间！");
                    return;
                }
                isRadar8x8Mode = false;
                GetRadar4T4Datafrommf4(input, output, start, end,CarData,CarSRSYawData);
            }

            if (loadedFilesDock) {
                           loadedFilesDock->updateRadarMF4(QFileInfo(input).fileName());
                           // 如果希望显示完整路径，改成 input
                           // loadedFilesDock->updateRadarMF4(input);
                       }

                       QMessageBox::information(this, "转换完成", "Radar 4x4 MF4 转换完成！");
        }
    }else if(ui->signal8_8->isChecked()){
        clearHighlightedPoints();
        multiFrames.clear();
        frames8x8.clear();   // 确保清理其他模式（如8x8）的帧数据

            // 1. 安全清空主点云指针 (PCL 数据)
            if (satecloud) {
                satecloud.reset();
            }

            // 2. 移除 VTK 渲染 Actor (屏幕显示)
            if (pointCloudActor) {
                mainRenderer->RemoveActor(pointCloudActor);
                pointCloudActor = nullptr; // 重置指针，确保 displayPointCloud() 重建管线
            }

        MdfExtractDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            QString input = dlg.inputFile();
            QString output = dlg.outputFile();

            if (input.isEmpty() || output.isEmpty()) {
                QMessageBox::warning(this, "错误", "请输入有效的输入/输出文件路径！");
                return;
            }



            if (dlg.isAllData()) {
                // 全部转换，忽略时间
                // 传入cardata std::vector<TimestampedCarData> CarData
                //GetRadar4T4Datafrommf4(input, output, 0.0, 0.0);
                // frames8x8 =GetRadar8T8Datafrommf4(input, output, 0.0, 0.0);
                QElapsedTimer tRead, tParse, tConvert;
                tRead.start();
                frames8x8 =GetRadar8T8Datafrommf4byfiles(input, output, 0.0, 0.0);
                std::cout << "Total:" << tRead.elapsed() << "ms"<<std::endl;
            } else {
                double start = dlg.startTime();
                double end   = dlg.endTime();

                if (end <= start) {
                    QMessageBox::warning(this, "错误", "结束时间必须大于开始时间！");
                    return;
                }

                QElapsedTimer tRead, tParse, tConvert;
                tRead.start();

                // frames8x8  = GetRadar8T8Datafrommf4(input, output, start, end);
                frames8x8  = GetRadar8T8Datafrommf4byfiles(input, output, start, end);

                std::cout << "Total:" << tRead.elapsed() << "ms"<<std::endl;
            }
            if (!frames8x8.empty()){
                multiFrames.clear();

                isRadar8x8Mode = true;

                for (int i = 0; i < frames8x8.size(); ++i) {
                                auto& frame = frames8x8[i];
                                auto cloud = frame.Radar8T8Point.cloud;

                                // 确保 cloud 存在且非空
                                if (cloud && !cloud->points.empty()) {
                                    // 🌟 假设：底层 GetRadar8T8Datafrommf4 或 convertPointCloud2ToPCL
                                    // 已经确保了 cloud 中所有点的 frameIndex == i
                                    multiFrames.push_back(cloud);
                                }
                            }

                currentFrameIndex = 0;
                isPlaying = false;
                // auto &frame = frames8x8[0];
                // satecloud = frame.Radar8T8Point.cloud;

                if (!multiFrames.empty()) {
                                satecloud = multiFrames[0];
                            } else {
                                QMessageBox::critical(this, "加载失败", "8x8 帧列表为空或点云为空！");
                                return;
                            }
                auto &firstFrame = frames8x8[0];

                if (pointCloudActor) {
                    mainRenderer->RemoveActor(pointCloudActor);
                    pointCloudActor = nullptr; // 强制重置 Actor
                }

                isFirstFrame = true;
                displayPointCloud();
                frameBuffer.clear();
                lastAccumulatedCloud = nullptr;
                rdMapPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
                showRDMap(rdMapPlot, firstFrame.Radar8T8Point.rdmap.data());
                pcMinMax = calculate8T8MinMax(frames8x8); // 假设有这个函数
                initSlidersFromPointCloud(pcMinMax.get());

                if (loadedFilesDock) {
                                   loadedFilesDock->updateRadarMF4(QFileInfo(input).fileName());
                               }

                QMessageBox::information(this, "加载成功",
                                         QString("加载了 %1 帧 8×8 点云，显示第1帧").arg(frames8x8.size()));

                ui->frameSlider->setMinimum(0);
                ui->frameSlider->setMaximum(static_cast<int>(multiFrames.size()) - 1);
                ui->frameSlider->setValue(0);
                ui->frameSlider->setSingleStep(1);
                ui->frameSlider->setPageStep(1);

                ui->pointstarttime->setText(QString::fromStdString(
                    convertTimestamp(firstFrame.start_time_ns + firstFrame.timestamp_ns)));
                ui->pointruntime->setText(QString::number(firstFrame.indexno ));
                double seconds = frames8x8.back().timestamp_ns / 1000000000.0;
                ui->pointendtime->setText(QString::number(seconds, 'f', 3) + " s");
            }


        }
    }
    /*
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Select mf4 file",
        "",
        "mf4 file (*.mf4)"
        );
    if (filename.isEmpty())
        return;
    QString savePath = QFileDialog::getSaveFileName(this , "Save Pointdata to bin", "", "bin 文件 (*.bin)");
    if (savePath.isEmpty())
        return;

    GetRadar4T4Datafrommf4(filename,savePath);
    */



}


void MainWindow::clearHighlightedPoints()
{

    clearMultiFrames();

    ui->pointstarttime->setText("");
    ui->pointendtime->setText("");
    ui->pointruntime->setText("");

    // 1. 清空已选 ID
    selectedPointIds.clear();

    // 2. 清空高亮点数据
    vtkNew<vtkPoints> newPoints; // 空的 points
    highlightedPoints->DeepCopy(newPoints);
    highlightedPoints->Modified();
    highlightedPolyData->SetPoints(highlightedPoints);

    // 3. 更新 glyph
    if (auto* glyphFilter = dynamic_cast<vtkVertexGlyphFilter*>(
            highlightedMapper->GetInputConnection(0, 0)->GetProducer()))
    {
        glyphFilter->Update();
    }

    // 4. 清空 UI 里的 comboBox 和 label
    ui->comboBoxSelectedPoints->clear();
    ui->labelPointInfo->clear();

    // 5. 强制刷新渲染
    ui->qvtkWidget->renderWindow()->Render();// 或者你对应的 vtkWidget
}

void MainWindow::on_pushButton_clicked()
{
    CarData.clear();
    CarSRSYawData.clear();
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Select mf4 Road recoder file",
        "",
        "mf4 file (*.mf4)"
        );
    if (filename.isEmpty())
        return;
    try {
        //std::vector<TimestampedCarData> CarData;
        if (ui->signal8_8->isChecked()) {
            CarData = readallmf4CarData8T8(filename.toStdString());

            CarSRSYawData=readallmf4CarSRSYawRate8T8(filename.toStdString());
            // qDebug()<<CarData.size();
        }else{

            CarData = readallmf4CarData(filename.toStdString());

            CarSRSYawData=readallmf4CarSRSYawRate(filename.toStdString());
        }


        // if (multiFrames.empty()) return;
        if (!CarData.empty() && !CarSRSYawData.empty()) {
            QString msg = QString("Loaded %1 car data entries and %2 SRS/Yaw entries.")
                                .arg(CarData.size())
                                .arg(CarSRSYawData.size());
            QMessageBox::information(this, "Load Complete", msg);

            uint64_t pointTime = 0;

            if (isRadar8x8Mode) {
                if (!frames8x8.empty() && currentFrameIndex < frames8x8.size()) {
                    pointTime = frames8x8[currentFrameIndex].timestamp_ns;
                } else {
                    // 没有点云帧时，取车辆数据的第一条时间
                    pointTime = CarData.front().timestamp_ns;
                }
            } else {
                if (!frames.empty() && currentFrameIndex < frames.size()) {
                    pointTime = frames[currentFrameIndex].start_time_ns + frames[currentFrameIndex].timestamp_ns;
                } else {
                    pointTime = CarData.front().start_time_ns + CarData.front().timestamp_ns;
                }
            }

            updateCarDataDisplay(pointTime);
            if (loadedFilesDock) {
                            loadedFilesDock->updateCarDataMF4(QFileInfo(filename).fileName());
                            // 如果想显示完整路径，可以直接传 filename
                            // loadedFilesDock->updateCarDataMF4(filename);
                        }
        } else {
            QMessageBox::warning(this, "Load Failed", "No valid car data found in this file.");
        }
    } catch (const std::exception &e) {
           QMessageBox::warning(this, "Load Failed", QString("读取 CarData MF4 文件失败：%1").arg(e.what()));
       } catch (...) {
           QMessageBox::warning(this, "Load Failed", "读取 CarData MF4 文件出现未知错误！");
       }

}

void MainWindow::updateCarDataDisplay(uint64_t PointTime){
    double speed_kph = 0.0; // 初始化，防止在 8x8 模式未触发时出现随机值
    double YawRate = 0.0;
    std::string displayTime; // 提出来，保证整个函数可见

    if (isRadar8x8Mode && m_current8x8Speed > -990.0f) {
            speed_kph = m_current8x8Speed;
            YawRate = m_current8x8YawRate;
            displayTime = convertTimestamp(PointTime);
        }
    else{

        if (CarData.empty()) return;


        // 找到时间戳最接近的 CarData
        int carIndex = 0;
        if (isRadar8x8Mode) {
                // 8x8 绝对时间
                while (carIndex + 1 < CarData.size() &&
                       std::llabs(CarData[carIndex + 1].timestamp_ns - PointTime) <
                       std::llabs(CarData[carIndex].timestamp_ns - PointTime)) {
                    carIndex++;
                }
            } else {
        while (carIndex + 1 < CarData.size() &&
               std::llabs(CarData[carIndex + 1].timestamp_ns+CarData[carIndex + 1].start_time_ns - PointTime) <
                   std::llabs(CarData[carIndex].timestamp_ns+CarData[carIndex].start_time_ns  - PointTime)) {
            carIndex++;
        }
        }

        int YawIndex=0;
        if (isRadar8x8Mode) {
                while (YawIndex + 1 < CarSRSYawData.size() &&
                       std::llabs(CarSRSYawData[YawIndex + 1].timestamp_ns - PointTime) <
                       std::llabs(CarSRSYawData[YawIndex].timestamp_ns - PointTime)) {
                    YawIndex++;
                }
            } else {
        while (YawIndex + 1 < CarSRSYawData.size() &&
               std::llabs(CarSRSYawData[YawIndex + 1].timestamp_ns+CarSRSYawData[YawIndex + 1].start_time_ns - PointTime) <
                   std::llabs(CarSRSYawData[YawIndex].timestamp_ns+CarSRSYawData[YawIndex].start_time_ns  - PointTime)) {
            YawIndex++;
        }
        }

        double speed = CarData[carIndex].data;
        YawRate=CarSRSYawData[YawIndex].data;
        speed_kph = speed;
        displayTime =convertTimestamp(CarData[carIndex].start_time_ns+CarData[carIndex].timestamp_ns);
    }


            // 原始速度（km/h）
    double speed_mps = speed_kph / 3.6;    // 换算为 m/s


    //ui->frameLabel->setText(QString("Frame: %1 ").arg(carIndex));
    // 使用更紧凑的 HTML 结构
    QString speedHtml = QString(
        "<html>"
        "<span style='font-size:14px; color:#FFD700;'>SPD </span>"
        "<span style='font-size:26px; color:#00FFCC; font-weight:bold;'>%1</span>"
        "<span style='font-size:14px; color:#00FFCC;'> km/h</span>"
        "<span style='font-size:18px; color:#666666;'> | </span>" // 分隔符
        "<span style='font-size:20px; color:#00FFCC; font-weight:bold;'>%2</span>"
        "<span style='font-size:14px; color:#00FFCC;'> m/s</span>"
        "</html>"
    ).arg(speed_kph, 0, 'f', 1).arg(speed_mps, 0, 'f', 2);


        // 偏航角速率显示
        QString yawHtml = QString(
            "<html>"
            "<span style='font-size:14px; color:#FFD700; font-weight:normal;'>YAW </span>"
            "<span style='font-size:26px; color:#00FFCC; font-weight:bold;'>%1</span>"
            "<span style='font-size:14px; color:#00FFCC; font-weight:normal;'> deg/s</span>"
            "</html>"
        ).arg(YawRate, 0, 'f', 2);

        ui->speedLabel->setText(speedHtml);
        ui->YawRate->setText(yawHtml);
        updateInfoPanelPosition();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);



    updateInfoPanelPosition();
     /*
    if (ui->infoPanel && ui->qvtkWidget) {


        ui->infoPanel->setStyleSheet(
            "background-color: rgba(0,0,0,0);"  // 背景透明
            "color: black;"
            );

        ui->infoPanel->setAttribute(Qt::WA_TranslucentBackground);
        QString labelStyle = "color: black; font-weight: bold; font-size: 16px; font-family: 'Consolas';";
        ui->speedLabel->setStyleSheet(labelStyle);
        ui->YawRate->setStyleSheet(labelStyle);


        int x = ui->qvtkWidget->width() - ui->infoPanel->width() - 10; // 右边距10px
        int y = 10; // 上边距10px
        ui->infoPanel->move(x, y);
        //ui->infoPanel->raise();
    }
*/
}

void MainWindow::updateInfoPanelPosition()
{
    // if (ui->infoPanel && ui->qvtkWidget) {
    //     ui->infoPanel->raise();
    //     int x = ui->qvtkWidget->width() - ui->infoPanel->width() - 5; // 右边距10px
    //     int y = 5; // 上边距10px
    //     ui->infoPanel->move(x, y);
    //     ui->infoPanel->show();
    // }
    if (ui->infoPanel && ui->qvtkWidget) {
            // 1. 解除之前的任何固定尺寸限制，允许控件根据内容变大
            ui->infoPanel->setMinimumSize(0, 0);
            ui->infoPanel->setMaximumSize(16777215, 16777215); // QWIDGETSIZE_MAX

            // 2. 强制 Qt 立即根据内部 Layout 重新计算长宽
            ui->infoPanel->adjustSize();

            int margin = 20;
            // 3. 动态计算 X 坐标：父宽 - 面板实测宽 - 边距
            // 这样面板变宽时，它会自动向左延伸，而右边始终对齐
            int x = ui->qvtkWidget->width() - ui->infoPanel->width() - margin;
            int y = margin;

            ui->infoPanel->move(x, y);
            ui->infoPanel->raise();
        }
}




void MainWindow::setupSpanSliders() {
    // 每个 slider 绑定 min/max 对应的 QLineEdit
    auto bindSlider = [this](QxtSpanSlider* slider, QLineEdit* minEdit, QLineEdit* maxEdit){
        slider->setProperty("minLineEdit", QVariant::fromValue<QObject*>(minEdit));
        slider->setProperty("maxLineEdit", QVariant::fromValue<QObject*>(maxEdit));

        connect(slider, &QxtSpanSlider::lowerValueChanged, this, &MainWindow::spanSliderValueChanged);
        connect(slider, &QxtSpanSlider::upperValueChanged, this, &MainWindow::spanSliderValueChanged);
    };

    bindSlider(ui->rangeslider, ui->rangemin, ui->rangemax);
    bindSlider(ui->dopplerSpeedslider, ui->dopplerSpeedmin, ui->dopplerSpeedmax);
    bindSlider(ui->powerdBslider, ui->powerdBmin, ui->powerdBmax);
    bindSlider(ui->SNRdBslider, ui->SNRdBmin, ui->SNRdBmax);
    bindSlider(ui->Q_aziSlider, ui->Q_azimin, ui->Q_azimax);
    bindSlider(ui->Q_eleSlider, ui->Q_elemin, ui->Q_elemax);
    bindSlider(ui->azimuthAngSlider, ui->azimuthAngmin, ui->azimuthAngmax);
    bindSlider(ui->eleAngSlider, ui->eleAngmin, ui->eleAngmax);
    bindSlider(ui->radVelAbsSlider, ui->radVelAbsmin, ui->radVelAbsmax);
    bindSlider(ui->rcsdBSlider, ui->rcsdBmin, ui->rcsdBmax);

}

void MainWindow::spanLineEditChanged(const QString &text)
{
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
    if (!edit) return;

    bool ok;
    double realValue = text.toDouble(&ok);   // 获取浮点输入
    if (!ok) return;

    // 遍历所有 slider，找出对应的 slider
    QList<QxtSpanSlider*> sliders = findChildren<QxtSpanSlider*>();
    for (QxtSpanSlider* slider : sliders) {
        if (!sliderMappings.contains(slider)) continue;

        const SpanSliderMapping& mapping = sliderMappings[slider];
        QLineEdit* minEdit = qobject_cast<QLineEdit*>(slider->property("minLineEdit").value<QObject*>());
        QLineEdit* maxEdit = qobject_cast<QLineEdit*>(slider->property("maxLineEdit").value<QObject*>());

        if (edit == minEdit) {
            slider->setLowerValue(mapping.toInt(realValue));  // 转换为 int
        } else if (edit == maxEdit) {
            slider->setUpperValue(mapping.toInt(realValue));
        }
    }
}


void MainWindow::spanSliderValueChanged(int)
{
    QxtSpanSlider* slider = qobject_cast<QxtSpanSlider*>(sender());
    if (!slider) return;
    if (!sliderMappings.contains(slider)) return;

    const SpanSliderMapping& mapping = sliderMappings[slider];

    double lowerReal = mapping.toReal(slider->lowerValue());
    double upperReal = mapping.toReal(slider->upperValue());

    QLineEdit* minEdit = qobject_cast<QLineEdit*>(slider->property("minLineEdit").value<QObject*>());
    QLineEdit* maxEdit = qobject_cast<QLineEdit*>(slider->property("maxLineEdit").value<QObject*>());

    if (minEdit) minEdit->setText(QString::number(lowerReal, 'f', 2));
    if (maxEdit) maxEdit->setText(QString::number(upperReal, 'f', 2));
}



void MainWindow::initSlidersFromPointCloud(const PointCloudDataminMax* data)
{
    auto initSlider = [&](QxtSpanSlider* slider, double defaultMin, double defaultMax,
                          double defaultLower, double defaultUpper,
                          double pcMin = 0, double pcMax = 0, bool hasData = false) {

        SpanSliderMapping mapping;
        mapping.updateRange(hasData ? pcMin : defaultMin, hasData ? pcMax : defaultMax);

        double lowerVal = hasData ? pcMin : defaultLower;
        double upperVal = hasData ? pcMax : defaultUpper;

        slider->setMinimum(mapping.intMin);
        slider->setMaximum(mapping.intMax);
        slider->setLowerValue(mapping.toInt(lowerVal));
        slider->setUpperValue(mapping.toInt(upperVal));

        sliderMappings[slider] = mapping;

        // 更新绑定的 QLineEdit
        if (slider->property("minLineEdit").isValid() &&
            slider->property("maxLineEdit").isValid()) {
            QLineEdit* minEdit = qobject_cast<QLineEdit*>(slider->property("minLineEdit").value<QObject*>());
            QLineEdit* maxEdit = qobject_cast<QLineEdit*>(slider->property("maxLineEdit").value<QObject*>());
            if (minEdit) minEdit->setText(QString::number(lowerVal, 'f', 2));
            if (maxEdit) maxEdit->setText(QString::number(upperVal, 'f', 2));
        }
    };

    bool hasPointCloud = (data != nullptr);

    initSlider(ui->rangeslider, 0.0, 300.0, 0.0, 300.0,
               hasPointCloud ? data->rangeMin : 0.0,
               hasPointCloud ? data->rangeMax : 300.0, hasPointCloud);

    initSlider(ui->dopplerSpeedslider, -50.0, 50.0, -50.0, 50.0,
               hasPointCloud ? data->dopplerSpeedMin : -50.0,
               hasPointCloud ? data->dopplerSpeedMax : 50.0, hasPointCloud);

    initSlider(ui->powerdBslider, 50.0, 200.0, 50.0, 200.0,
               hasPointCloud ? data->powerDBMin : 50.0,
               hasPointCloud ? data->powerDBMax : 200.0, hasPointCloud);

    initSlider(ui->SNRdBslider, 0.0, 100.0, 0.0, 100.0,
               hasPointCloud ? data->SNRdBMin : 0.0,
               hasPointCloud ? data->SNRdBMax : 100.0, hasPointCloud);

    initSlider(ui->Q_aziSlider, 0.0, 1.0, 0.0, 1.0,
               hasPointCloud ? data->Q_aziMin : 0.0,
               hasPointCloud ? data->Q_aziMax : 1.0, hasPointCloud);
    initSlider(ui->Q_eleSlider, 0.0, 1.0, 0.0, 1.0,
               hasPointCloud ? data->Q_eleMin : 0.0,
               hasPointCloud ? data->Q_eleMax : 1.0, hasPointCloud);

    initSlider(ui->azimuthAngSlider, -100.0, 100.0, -100.0, 100.0,
               hasPointCloud ? data->azimuthAngMin : -100.0,
               hasPointCloud ? data->azimuthAngMax : 100.0, hasPointCloud);
    initSlider(ui->eleAngSlider, -50.0, 50.0, -50.0, 50.0,
               hasPointCloud ? data->eleAngMin : -50.0,
               hasPointCloud ? data->eleAngMax : 50.0, hasPointCloud);

    initSlider(ui->radVelAbsSlider, -50.0, 50.0, -50.0, 50.0,
               hasPointCloud ? data->radVelAbsMin : -50.0,
               hasPointCloud ? data->radVelAbsMax : 50.0, hasPointCloud);
    initSlider(ui->rcsdBSlider, -50.0, 100.0, -50.0, 100.0,
               hasPointCloud ? data->rcsdBMin : -50.0,
               hasPointCloud ? data->rcsdBMax : 100.0, hasPointCloud);
    // 其他 slider 同理..
}

void MainWindow::onResetFilterClicked()
{
    // 调用已有初始化逻辑
    if(pcMinMax){
        initSlidersFromPointCloud(pcMinMax.get());
    }else{
        initSlidersFromPointCloud(nullptr);
    }

    currentFilter.enabled = false;

    currentFilter.detValidMode = FilterCriteria::All;
    // 设置 radioAll 为选中状态
    ui->radioall->setChecked(true);
    ui->radio0->setChecked(false);
    ui->radio1->setChecked(false);
    onApplyFilterClicked();
    displayPointCloud();
   // QMessageBox::information(this, "提示", "筛选条件已重置，恢复默认范围");
  //  qDebug()  << "筛选条件已重置，恢复默认范围"<<endl;

}

// 应用筛选按钮
void MainWindow::onApplyFilterClicked()
{

    if (!ui->rangeslider) return;

    currentFilter.enabled = true;

    auto setRange = [&](QxtSpanSlider* slider, double& minVal, double& maxVal) {
        if (!sliderMappings.contains(slider)) return;
        const auto& mapping = sliderMappings[slider];
        minVal = mapping.toReal(slider->lowerValue());
        maxVal = mapping.toReal(slider->upperValue());

        qDebug() << slider->objectName() << ":" << minVal << "~" << maxVal;
    };

    setRange(ui->rangeslider, currentFilter.rangeMin, currentFilter.rangeMax);
    setRange(ui->dopplerSpeedslider, currentFilter.dopplerMin, currentFilter.dopplerMax);
    setRange(ui->powerdBslider, currentFilter.powerdBMin, currentFilter.powerdBMax);
    setRange(ui->SNRdBslider, currentFilter.snrMin, currentFilter.snrMax);
    setRange(ui->Q_aziSlider, currentFilter.qAziMin, currentFilter.qAziMax);
    setRange(ui->Q_eleSlider, currentFilter.qEleMin, currentFilter.qEleMax);
    setRange(ui->azimuthAngSlider, currentFilter.aziAngMin, currentFilter.aziAngMax);
    setRange(ui->eleAngSlider, currentFilter.eleAngMin, currentFilter.eleAngMax);
    setRange(ui->radVelAbsSlider, currentFilter.radVelAbsMin, currentFilter.radVelAbsMax);
    setRange(ui->rcsdBSlider, currentFilter.rcsdBMin, currentFilter.rcsdBMax);

    if (ui->radioall->isChecked()) {
        currentFilter.detValidMode = FilterCriteria::All;
    } else if (ui->radio0->isChecked()) {
        currentFilter.detValidMode = FilterCriteria::Only0;
    } else if (ui->radio1->isChecked()) {
        currentFilter.detValidMode = FilterCriteria::Only1;
    }

    filterAppliedFlag=true;
    displayPointCloud();
    /*
    QList<QxtSpanSlider*> sliders = findChildren<QxtSpanSlider*>();
    for (QxtSpanSlider* slider : sliders) {
        if (!sliderMappings.contains(slider)) continue;

        const SpanSliderMapping& mapping = sliderMappings[slider];

        double lowerReal = mapping.toReal(slider->lowerValue());
        double upperReal = mapping.toReal(slider->upperValue());

        QString sliderName = slider->objectName();


        qDebug() << "item:" << sliderName
                 << "range:" << lowerReal << "~" << upperReal;

        // TODO: 在这里调用点云筛选逻辑
        // pointCloud.filterByRange(sliderName, lowerReal, upperReal);
    }*/
}


std::unique_ptr<PointCloudDataminMax> MainWindow::calculateMultiFrameMinMax(
    const std::vector<std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>>& multiFrames)
{
    if (multiFrames.empty()) return nullptr;

    bool hasPoint = false;
    PointCloudDataminMax minMax;

    // 初始化为极端值
    minMax.rangeMin = std::numeric_limits<double>::max();
    minMax.rangeMax = std::numeric_limits<double>::lowest();
    minMax.dopplerSpeedMin = std::numeric_limits<double>::max();
    minMax.dopplerSpeedMax = std::numeric_limits<double>::lowest();
    minMax.powerDBMin = std::numeric_limits<double>::max();
    minMax.powerDBMax = std::numeric_limits<double>::lowest();
    minMax.SNRdBMin = std::numeric_limits<double>::max();
    minMax.SNRdBMax = std::numeric_limits<double>::lowest();
    minMax.Q_aziMin = std::numeric_limits<double>::max();
    minMax.Q_aziMax = std::numeric_limits<double>::lowest();
    minMax.Q_eleMin = std::numeric_limits<double>::max();
    minMax.Q_eleMax = std::numeric_limits<double>::lowest();
    minMax.azimuthAngMin = std::numeric_limits<double>::max();
    minMax.azimuthAngMax = std::numeric_limits<double>::lowest();
    minMax.eleAngMin = std::numeric_limits<double>::max();
    minMax.eleAngMax = std::numeric_limits<double>::lowest();
    minMax.radVelAbsMin = std::numeric_limits<double>::max();
    minMax.radVelAbsMax = std::numeric_limits<double>::lowest();
    minMax.rcsdBMin = std::numeric_limits<double>::max();
    minMax.rcsdBMax = std::numeric_limits<double>::lowest();

    for (const auto& frameCloud : multiFrames) {
        if (!frameCloud || frameCloud->empty()) continue;

        for (const auto& p : frameCloud->points) {
            hasPoint = true;
            minMax.rangeMin = std::min(minMax.rangeMin, static_cast<double>(p.range));
            minMax.rangeMax = std::max(minMax.rangeMax, static_cast<double>(p.range));

            minMax.dopplerSpeedMin = std::min(minMax.dopplerSpeedMin, static_cast<double>(p.dopplerSpeed));
            minMax.dopplerSpeedMax = std::max(minMax.dopplerSpeedMax, static_cast<double>(p.dopplerSpeed));

            minMax.powerDBMin = std::min(minMax.powerDBMin, static_cast<double>(p.powerdB));
            minMax.powerDBMax = std::max(minMax.powerDBMax, static_cast<double>(p.powerdB));


            minMax.SNRdBMin = std::min(minMax.SNRdBMin, static_cast<double>(p.SNRdB));
            minMax.SNRdBMax = std::max(minMax.SNRdBMax, static_cast<double>(p.SNRdB));

            minMax.Q_aziMin = std::min(minMax.Q_aziMin, static_cast<double>(p.Q_azi));
            minMax.Q_aziMax = std::max(minMax.Q_aziMax, static_cast<double>(p.Q_azi));

            minMax.Q_eleMin = std::min(minMax.Q_eleMin, static_cast<double>(p.Q_ele));
            minMax.Q_eleMax = std::max(minMax.Q_eleMax, static_cast<double>(p.Q_ele));

            minMax.azimuthAngMin = std::min(minMax.azimuthAngMin, static_cast<double>(p.azimuthAng));
            minMax.azimuthAngMax = std::max(minMax.azimuthAngMax, static_cast<double>(p.azimuthAng));

            minMax.eleAngMin = std::min(minMax.eleAngMin, static_cast<double>(p.eleAng));
            minMax.eleAngMax = std::max(minMax.eleAngMax, static_cast<double>(p.eleAng));

            minMax.radVelAbsMin = std::min(minMax.radVelAbsMin, static_cast<double>(p.radVelAbs));
            minMax.radVelAbsMax = std::max(minMax.radVelAbsMax, static_cast<double>(p.radVelAbs));

            minMax.rcsdBMin = std::min(minMax.rcsdBMin, static_cast<double>(p.rcsdB));
            minMax.rcsdBMax = std::max(minMax.rcsdBMax, static_cast<double>(p.rcsdB));
        }
    }

    if (!hasPoint) return nullptr;
    // 加入小幅偏移

    const double factor = 0.01; // 1% 缓冲，可根据需要调整
    auto applyBuffer = [factor](double& minVal, double& maxVal) {
        double range = maxVal - minVal;
        minVal -= range * factor;
        maxVal += range * factor;
    };
    applyBuffer(minMax.rangeMin, minMax.rangeMax);
    applyBuffer(minMax.dopplerSpeedMin, minMax.dopplerSpeedMax);
    applyBuffer(minMax.powerDBMin, minMax.powerDBMax);
    applyBuffer(minMax.SNRdBMin, minMax.SNRdBMax);
    applyBuffer(minMax.Q_aziMin, minMax.Q_aziMax);
    applyBuffer(minMax.Q_eleMin, minMax.Q_eleMax);
    applyBuffer(minMax.azimuthAngMin, minMax.azimuthAngMax);
    applyBuffer(minMax.eleAngMin, minMax.eleAngMax);
    applyBuffer(minMax.radVelAbsMin, minMax.radVelAbsMax);
    applyBuffer(minMax.rcsdBMin, minMax.rcsdBMax);

    return std::make_unique<PointCloudDataminMax>(minMax);
}

std::unique_ptr<PointCloudDataminMax> MainWindow::calculate8T8MinMax(
    const std::vector<pointandtimedata8x8>& frames8x8)
{
    // 检查输入是否为空
    if (frames8x8.empty()) return nullptr;

    bool hasPoint = false;
    PointCloudDataminMax minMax;

    // 初始化为极端值 (与您的 4x4 函数保持一致)
    minMax.rangeMin = std::numeric_limits<double>::max();
    minMax.rangeMax = std::numeric_limits<double>::lowest();
    minMax.dopplerSpeedMin = std::numeric_limits<double>::max();
    minMax.dopplerSpeedMax = std::numeric_limits<double>::lowest();
    minMax.powerDBMin = std::numeric_limits<double>::max();
    minMax.powerDBMax = std::numeric_limits<double>::lowest();
    minMax.SNRdBMin = std::numeric_limits<double>::max();
    minMax.SNRdBMax = std::numeric_limits<double>::lowest();
    minMax.Q_aziMin = std::numeric_limits<double>::max();
    minMax.Q_aziMax = std::numeric_limits<double>::lowest();
    minMax.Q_eleMin = std::numeric_limits<double>::max();
    minMax.Q_eleMax = std::numeric_limits<double>::lowest();
    minMax.azimuthAngMin = std::numeric_limits<double>::max();
    minMax.azimuthAngMax = std::numeric_limits<double>::lowest();
    minMax.eleAngMin = std::numeric_limits<double>::max();
    minMax.eleAngMax = std::numeric_limits<double>::lowest();
    minMax.radVelAbsMin = std::numeric_limits<double>::max();
    minMax.radVelAbsMax = std::numeric_limits<double>::lowest();
    minMax.rcsdBMin = std::numeric_limits<double>::max();
    minMax.rcsdBMax = std::numeric_limits<double>::lowest();

    // 遍历 8x8 结构体帧
    for (const auto& frame : frames8x8) {
        // 提取 PointCloud 指针
        const auto& frameCloud = frame.Radar8T8Point.cloud;

        if (!frameCloud || frameCloud->empty()) continue;

        // 遍历点云中的点
        for (const auto& p : frameCloud->points) {
            hasPoint = true;

            // 使用与 4x4 函数相同的逻辑进行 Min/Max 更新
            minMax.rangeMin = std::min(minMax.rangeMin, static_cast<double>(p.range));
            minMax.rangeMax = std::max(minMax.rangeMax, static_cast<double>(p.range));

            minMax.dopplerSpeedMin = std::min(minMax.dopplerSpeedMin, static_cast<double>(p.dopplerSpeed));
            minMax.dopplerSpeedMax = std::max(minMax.dopplerSpeedMax, static_cast<double>(p.dopplerSpeed));

            minMax.powerDBMin = std::min(minMax.powerDBMin, static_cast<double>(p.powerdB));
            minMax.powerDBMax = std::max(minMax.powerDBMax, static_cast<double>(p.powerdB));

            minMax.SNRdBMin = std::min(minMax.SNRdBMin, static_cast<double>(p.SNRdB));
            minMax.SNRdBMax = std::max(minMax.SNRdBMax, static_cast<double>(p.SNRdB));

            minMax.Q_aziMin = std::min(minMax.Q_aziMin, static_cast<double>(p.Q_azi));
            minMax.Q_aziMax = std::max(minMax.Q_aziMax, static_cast<double>(p.Q_azi));

            minMax.Q_eleMin = std::min(minMax.Q_eleMin, static_cast<double>(p.Q_ele));
            minMax.Q_eleMax = std::max(minMax.Q_eleMax, static_cast<double>(p.Q_ele));

            minMax.azimuthAngMin = std::min(minMax.azimuthAngMin, static_cast<double>(p.azimuthAng));
            minMax.azimuthAngMax = std::max(minMax.azimuthAngMax, static_cast<double>(p.azimuthAng));

            minMax.eleAngMin = std::min(minMax.eleAngMin, static_cast<double>(p.eleAng));
            minMax.eleAngMax = std::max(minMax.eleAngMax, static_cast<double>(p.eleAng));

            minMax.radVelAbsMin = std::min(minMax.radVelAbsMin, static_cast<double>(p.radVelAbs));
            minMax.radVelAbsMax = std::max(minMax.radVelAbsMax, static_cast<double>(p.radVelAbs));

            minMax.rcsdBMin = std::min(minMax.rcsdBMin, static_cast<double>(p.rcsdB));
            minMax.rcsdBMax = std::max(minMax.rcsdBMax, static_cast<double>(p.rcsdB));
        }
    }

    if (!hasPoint) return nullptr;

    // --- 应用缓冲偏移 (与您的 4x4 函数保持一致) ---

    const double factor = 0.01; // 1% 缓冲
    auto applyBuffer = [factor](double& minVal, double& maxVal) {
        double range = maxVal - minVal;
        // 避免范围为零或负数时的错误计算
        if (range > 0) {
            minVal -= range * factor;
            maxVal += range * factor;
        } else if (range == 0) {
            // 如果最小值和最大值相同，增加一个微小的固定范围
            double epsilon = 1.0;
            minVal -= epsilon;
            maxVal += epsilon;
        }
    };

    applyBuffer(minMax.rangeMin, minMax.rangeMax);
    applyBuffer(minMax.dopplerSpeedMin, minMax.dopplerSpeedMax);
    applyBuffer(minMax.powerDBMin, minMax.powerDBMax);
    applyBuffer(minMax.SNRdBMin, minMax.SNRdBMax);
    applyBuffer(minMax.Q_aziMin, minMax.Q_aziMax);
    applyBuffer(minMax.Q_eleMin, minMax.Q_eleMax);
    applyBuffer(minMax.azimuthAngMin, minMax.azimuthAngMax);
    applyBuffer(minMax.eleAngMin, minMax.eleAngMax);
    applyBuffer(minMax.radVelAbsMin, minMax.radVelAbsMax);
    applyBuffer(minMax.rcsdBMin, minMax.rcsdBMax);

    return std::make_unique<PointCloudDataminMax>(minMax);
}

void MainWindow::saveSettings()
{
    QSettings settings;

        // 1. 保存窗口几何尺寸（位置、最大化状态等）
        settings.setValue("mainWindow/geometry", saveGeometry());

        // 2. 保存所有 Dock 的布局（这是最关键的：包含位置、比例、显示/隐藏）
        settings.setValue("mainWindow/windowState", saveState());

        // 3. (可选) 如果你一定要单独存 CheckBox 状态，可以保留，但逻辑上其实 saveState 已包含
        settings.setValue("view/Pointcloutviewer", ui->Pointcloutviewer->isChecked());
        settings.setValue("view/DisplayVideo", ui->DisplayVideo->isChecked());
        settings.setValue("view/RDimage", ui->RDimage->isChecked());
        settings.setValue("view/selectpiontdata", ui->selectpiontdata->isChecked());
        settings.setValue("view/pointfilter", ui->pointfilter->isChecked());

        qDebug() << "Settings saved successfully.";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadSettings()
{
    QSettings settings;



    restoreGeometry(settings.value("mainWindow/geometry").toByteArray());

    QByteArray state = settings.value("mainWindow/windowState").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);  // 恢复用户上次布局
    }

    auto syncCheckbox = [](QCheckBox* cb, QDockWidget* dock) {
            cb->blockSignals(true);
            cb->setChecked(dock->isVisible());
            cb->blockSignals(false);
        };

        syncCheckbox(ui->Pointcloutviewer, ui->PointCloudDock);
        syncCheckbox(ui->DisplayVideo, ui->Video);
        syncCheckbox(ui->RDimage, ui->RD);
        syncCheckbox(ui->selectpiontdata, ui->PointData);
        syncCheckbox(ui->pointfilter, ui->PointFilters);


}

void MainWindow::setupApplicationSettings()
{
    QCoreApplication::setOrganizationName("YourCompany");
    QCoreApplication::setApplicationName("PointCloudUI");
}


// mainwindow.cpp (实现槽函数)

void MainWindow::setUserXRange(int value)
{
    if (value <= 0) return;

    userXRange = static_cast<double>(value);

    // 只更新 ParallelScale，但不改 FocalPoint
    vtkCamera* camera = mainRenderer->GetActiveCamera();
    if (camera) {
        double scale = std::max(userXRange, userYRange);
        camera->SetParallelScale(scale);
        // 注意：不要再修改 focal point
    }

    // 网格和标签使用 userXRange/userYRange 绘制，十字不动
    AddGridAndLabelsDynamically();

    int index = ui->ViewupcomboBox->currentIndex();
    on_ViewupcomboBox_currentIndexChanged(index);
}

void MainWindow::setUserYRange(int value)
{
    if (value <= 0) return;

    userYRange = static_cast<double>(value);

    vtkCamera* camera = mainRenderer->GetActiveCamera();
    if (camera) {
        double scale = std::max(userXRange, userYRange);
        camera->SetParallelScale(scale);
    }

    AddGridAndLabelsDynamically();

    int index = ui->ViewupcomboBox->currentIndex();
    on_ViewupcomboBox_currentIndexChanged(index);
}


void MainWindow::setMaxFramesToAccumulate(int count)
{
    maxFramesToAccumulate = (count > 0) ? count : 1;
}



void MainWindow::on_XYAziInitial_clicked()
{
    ui->XYazimuspinBox->setValue(1);
    ui->XYazimuspinBox->setValue(0);

}




void MainWindow::on_XYZ45_clicked()
{
    // double azimuthDeg   = -66.9706;   // 绕 Z 轴旋转
    //    double elevationDeg = -18.3396;   // 绕 Right 旋转
    //    double rollDeg      = 22.9312;    // 沿视线旋转
    //    double distance     = 600.0;      // 相机到焦点距离
    //    double focalPoint[3]= {104.291, 25.077, 66.5499};  // 焦点

    //    // -----------------------------
    //    // 2️⃣ 设置相机
    //    // -----------------------------
    //    setCameraByAzElRollDistanceDirect(
    //        azimuthDeg,
    //        elevationDeg,
    //        distance,
    //        rollDeg,
    //        focalPoint
    //    );

    vtkCamera* camera = mainRenderer->GetActiveCamera();

        // 🌟 关键：回归中心。3D 视角通常不需要 WindowCenter 偏移
        camera->SetWindowCenter(0,0.8);

    // double position[3] = {-118.509 ,549.214 ,255.339};
    // double focalPoint[3] = {104.291, 25.077 ,66.5499};
    // double viewUp[3] = { 0.471938 ,-0.114266, 0.874195};
        double position[3] = {-200.326, 548.403 ,161.033};
        double focalPoint[3] = {22.4739, 24.2665, -27.7565};
        double viewUp[3] = { 0.471938 ,-0.114266, 0.874195};
        setCameraCustomAngle(position,focalPoint,viewUp);

        double camPos[3];
        camera->GetPosition(camPos);
        qDebug() << "Camera Position:" << camPos[0] << camPos[1] << camPos[2];

        double camFocal[3];
        camera->GetFocalPoint(camFocal);
        qDebug() << "Camera FocalPoint:" << camFocal[0] << camFocal[1] << camFocal[2];

        double camUp[3];
        camera->GetViewUp(camUp);
        qDebug() << "Camera ViewUp:" << camUp[0] << camUp[1] << camUp[2];


        // mainRenderer->ResetCameraClippingRange();
        //    AddGridAndLabelsDynamically();
        //    ui->qvtkWidget->renderWindow()->Render();
}


void MainWindow::setCameraCustomAngle(const double position[3],
                                      const double focalPoint[3],
                                      const double desiredViewUp[3])
{
    vtkCamera* camera = mainRenderer->GetActiveCamera();
       if (!camera) return;

       // 1️⃣ 设置 Position 和 FocalPoint
       camera->SetPosition(position);
       camera->SetFocalPoint(focalPoint);

       // 2️⃣ 计算视线方向
       double viewDir[3] = {
           focalPoint[0] - position[0],
           focalPoint[1] - position[1],
           focalPoint[2] - position[2]
       };
       vtkMath::Normalize(viewDir);

       // 3️⃣ 正交化 ViewUp
       double viewUp[3] = {desiredViewUp[0], desiredViewUp[1], desiredViewUp[2]};
       vtkMath::Normalize(viewUp);

       // 投影到与视线方向正交的平面上
       double dot = vtkMath::Dot(viewUp, viewDir);
       viewUp[0] -= dot * viewDir[0];
       viewUp[1] -= dot * viewDir[1];
       viewUp[2] -= dot * viewDir[2];
       vtkMath::Normalize(viewUp);

       camera->SetViewUp(viewUp);

       // 4️⃣ 刷新渲染
       mainRenderer->ResetCameraClippingRange();
       ui->qvtkWidget->renderWindow()->Render();

       // 5️⃣ 同步 ViewUp ComboBox（可选）
       // int index = ui->ViewupcomboBox->currentIndex();
       // on_ViewupcomboBox_currentIndexChanged(index);

       // 6️⃣ 打印最终参数，保证和屏幕显示一致
       // double pos[3], fp[3], vu[3];
       // camera->GetPosition(pos);
       // camera->GetFocalPoint(fp);
       // camera->GetViewUp(vu);

       // qDebug() << "Camera set with orthogonal ViewUp:";
       // qDebug() << " Position:" << pos[0] << pos[1] << pos[2];
       // qDebug() << " FocalPoint:" << fp[0] << fp[1] << fp[2];
       // qDebug() << " ViewUp:" << vu[0] << vu[1] << vu[2];
       // qDebug() << " Distance:" << camera->GetDistance();
       // qDebug() << " ViewAngle:" << camera->GetViewAngle();
}

void MainWindow::computeAzElRollFromCamera(
    const double position[3],
    const double focalPoint[3],
    const double viewUp[3],
    double &azimuthDeg,
    double &elevationDeg,
    double &rollDeg)
{
    double V[3] = {focalPoint[0]-position[0], focalPoint[1]-position[1], focalPoint[2]-position[2]};
    vtkMath::Normalize(V);

    double U[3] = {viewUp[0], viewUp[1], viewUp[2]};
    vtkMath::Normalize(U);

    double R[3];
    vtkMath::Cross(V, U, R);
    vtkMath::Normalize(R);

    double Up[3];
    vtkMath::Cross(R, V, Up);
    vtkMath::Normalize(Up);

    // Azimuth
    azimuthDeg = vtkMath::DegreesFromRadians(std::atan2(V[1], V[0]));

    // Elevation
    double xyLen = std::sqrt(V[0]*V[0] + V[1]*V[1]);
    elevationDeg = vtkMath::DegreesFromRadians(std::atan2(V[2], xyLen));

    // Roll
    double worldUp[3] = {0,0,1};
    rollDeg = vtkMath::DegreesFromRadians(std::atan2(vtkMath::Dot(R, worldUp), vtkMath::Dot(Up, worldUp)));
}

void MainWindow::setCameraByAzElRollDistance(
    double azimuthDeg,
    double elevationDeg,
    double distance,
    double rollDeg,
    const double focalPoint[3])
{
    vtkCamera* camera = mainRenderer->GetActiveCamera();
    if (!camera) return;

    // 1️⃣ 设置焦点
    camera->SetFocalPoint(focalPoint);

    // 2️⃣ 初始位置：先放在 +X 方向 distance 距离
    double position[3] = {
        focalPoint[0] + distance,
        focalPoint[1],
        focalPoint[2]
    };
    camera->SetPosition(position);

    // 3️⃣ 初始 ViewUp（随便先设一个方向，之后 Roll 会旋转）
    camera->SetViewUp(-1.0, 0.0, 0.0);

    // 4️⃣ 按鼠标交互顺序旋转
    camera->Azimuth(azimuthDeg);     // 绕 ViewUp 旋转
    camera->Elevation(elevationDeg); // 绕 Right 旋转
    camera->Roll(rollDeg);           // 沿视线旋转

    // 5️⃣ 刷新渲染
    mainRenderer->ResetCameraClippingRange();
    ui->qvtkWidget->renderWindow()->Render();

    // 6️⃣ 可选打印确认
    double pos[3], fp[3], vu[3];
    camera->GetPosition(pos);
    camera->GetFocalPoint(fp);
    camera->GetViewUp(vu);

    qDebug() << "Camera set by Azimuth/Elevation/Roll:";
    qDebug() << " Position:" << pos[0] << pos[1] << pos[2];
    qDebug() << " FocalPoint:" << fp[0] << fp[1] << fp[2];
    qDebug() << " ViewUp:" << vu[0] << vu[1] << vu[2];
    qDebug() << " Distance:" << camera->GetDistance();
    qDebug() << " ViewAngle:" << camera->GetViewAngle();
}
void MainWindow::setCameraByAzElRollDistanceDirect(
    double azimuthDeg,
    double elevationDeg,
    double distance,
    double rollDeg,
    const double focalPoint[3])
{
    vtkCamera* camera = mainRenderer->GetActiveCamera();
        if (!camera) return;

        double azRad = vtkMath::RadiansFromDegrees(azimuthDeg);
        double elRad = vtkMath::RadiansFromDegrees(elevationDeg);

        // 1️⃣ 修正映射逻辑
        // x_off 保持不变以维持 X 指向左上的趋势
        double x_off = distance * cos(elRad) * (-sin(azRad));
        // 🔴 关键修正：将 cos(azRad) 前的符号改为负号，反转 Y 轴在世界空间的位置
        double y_off = distance * cos(elRad) * (-cos(azRad));
        double z_off = distance * sin(elRad);

        double position[3] = {
            focalPoint[0] + x_off,
            focalPoint[1] + y_off,
            focalPoint[2] + z_off
        };

        camera->SetPosition(position);
        camera->SetFocalPoint(focalPoint);

        // 2️⃣ 正交化逻辑 (使用你要求的 {-1, 0, 0} 作为基准)
        double viewDir[3] = {
            focalPoint[0] - position[0],
            focalPoint[1] - position[1],
            focalPoint[2] - position[2]
        };
        vtkMath::Normalize(viewDir);

        // 🔴 关键修正：设置你要求的初始参考向上方向
        double desiredViewUp[3] = {-1.0, 0.0, 0.0};
        double viewUp[3] = {desiredViewUp[0], desiredViewUp[1], desiredViewUp[2]};

        // 正交化投影：确保 viewUp 垂直于视线方向
        double dot = vtkMath::Dot(viewUp, viewDir);
        viewUp[0] -= dot * viewDir[0];
        viewUp[1] -= dot * viewDir[1];
        viewUp[2] -= dot * viewDir[2];
        vtkMath::Normalize(viewUp);

        camera->SetViewUp(viewUp);

        // 3️⃣ 应用 Roll
        // 注意：在正交化之后使用 Roll，它是相对于当前计算出的 viewUp 的旋转
        camera->SetRoll(rollDeg);

        mainRenderer->ResetCameraClippingRange();
        ui->qvtkWidget->renderWindow()->Render();

        int index = ui->ViewupcomboBox->currentIndex();
        on_ViewupcomboBox_currentIndexChanged(index);
}

void MainWindow::on_prevFrameButton_clicked()
{
    int val = ui->frameSlider->value();
        if (val > 0) {
            ui->frameSlider->setValue(val - 1);
        }
}


void MainWindow::on_nextFrameButton_clicked()
{
    int val = ui->frameSlider->value();
        if (isRadar8x8Mode && val < static_cast<int>(frames8x8.size() - 1)) {
            ui->frameSlider->setValue(val + 1);
        } else if (!isRadar8x8Mode && val < static_cast<int>(multiFrames.size() - 1)) {
            ui->frameSlider->setValue(val + 1);
        }
}



void MainWindow::updateColorBar(const QString& arrayName, vtkLookupTable* lut)
{
    if (!pointCloudMapper || !pointCloudPolyData || !lut) return;

    pointCloudPolyData->GetPointData()
        ->SetActiveScalars(arrayName.toStdString().c_str());

    // 1. Mapper 配置
    pointCloudMapper->ScalarVisibilityOn();
    pointCloudMapper->SelectColorArray(arrayName.toStdString().c_str());
    pointCloudMapper->SetScalarModeToUsePointData();
    pointCloudMapper->SetColorModeToMapScalars();
    pointCloudMapper->SetLookupTable(lut);
    pointCloudMapper->UseLookupTableScalarRangeOn();

    // 2. 设置 LUT Range
    QString title = arrayName;
    if (arrayName == "height") {
        lut->SetRange(-1.0, 4.0);
        title = "Height (m)";
    }
    else if (arrayName == "radVelAbs") {
        lut->SetRange(-40.0, 40.0);
        title = "Speed (m/s)";
    }
    else if (arrayName == "RcsdB") {
        lut->SetRange(-10.0, 40.0);
        title = "RCS (dB)";
    }
    else if (arrayName == "SNRdB") {
        lut->SetRange(0.0, 50.0);
        title = "SNR";
    }
    else {
        if (auto array = pointCloudPolyData->GetPointData()->GetArray(arrayName.toStdString().c_str())) {
            double range[2]; array->GetRange(range);
            lut->SetRange(range);
        }
    }
    lut->Build();

    // 3. 创建/更新 ColorBar
    if (!scalarBarActor) {
        scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
        mainRenderer->AddActor2D(scalarBarActor);
    }

    scalarBarActor->SetLookupTable(lut);
    scalarBarActor->SetTitle(title.toStdString().c_str());
    scalarBarActor->SetVisibility(true);

    // 工业级样式设置
    scalarBarActor->SetOrientationToVertical();
    scalarBarActor->SetWidth(0.08);     // 窄而专业
    scalarBarActor->SetHeight(0.5);    // 高度大约占窗口 75%
    scalarBarActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    scalarBarActor->GetPositionCoordinate()->SetValue(0.92, 0.1); // 右侧偏右

    scalarBarActor->UnconstrainedFontSizeOn();
    scalarBarActor->SetTitleRatio(0.25); // 标题占比 25%
    scalarBarActor->SetNumberOfLabels(5); // 标签数量

    // 标题样式
    vtkTextProperty* titleProp = scalarBarActor->GetTitleTextProperty();
    titleProp->SetFontSize(12);
    titleProp->SetColor(0.2, 0.2, 0.2);   // 深灰
    titleProp->BoldOn();

    // 标签样式
    vtkTextProperty* labelProp = scalarBarActor->GetLabelTextProperty();
    labelProp->SetFontSize(10);
    labelProp->SetColor(0.2, 0.2, 0.2);   // 深灰
    labelProp->BoldOn();

    // 边框样式
    scalarBarActor->DrawFrameOff();
    scalarBarActor->GetFrameProperty()->SetColor(0.15, 0.15, 0.15); // 深灰边框

    // 4. 更新渲染
    pointCloudMapper->Update();
    ui->qvtkWidget->renderWindow()->Render();
}



vtkSmartPointer<vtkLookupTable> MainWindow::createRadarLUT() {
    vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();

    // 推荐配色逻辑 (Blue -> Black -> Orange)
    ctf->AddRGBPoint(-40.0, 0.0, 0.2, 0.8); // 深蓝
    ctf->AddRGBPoint(-10.0, 0.2, 0.6, 1.0); // 天蓝
    ctf->AddRGBPoint(0.0,   0.05, 0.05, 0.05); // 接近黑色的深灰 (静止)
    ctf->AddRGBPoint(10.0,  1.0, 0.4, 0.0); // 橙
    ctf->AddRGBPoint(40.0,  1.0, 0.9, 0.0); // 亮黄

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(256);
    lut->SetRange(-40.0, 40.0);

    for (int i = 0; i < 256; ++i) {
        double ratio = static_cast<double>(i) / 255.0;
        double val = -40.0 + ratio * 80.0;
        double rgb[3];
        ctf->GetColor(val, rgb);
        lut->SetTableValue(i, rgb[0], rgb[1], rgb[2], 1.0);
    }

    lut->Build();
    return lut;
}

vtkSmartPointer<vtkLookupTable> MainWindow::createRadarSNRLUT() {
    vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();

    // 推荐配色逻辑 (Blue -> Black -> Orange)
    ctf->AddRGBPoint(0, 0.0, 0.2, 0.8); // 深蓝
    ctf->AddRGBPoint(10, 0.2, 0.6, 1.0); // 天蓝
    ctf->AddRGBPoint(20,   0.05, 0.05, 0.05); // 接近黑色的深灰 (静止)
    ctf->AddRGBPoint(30,  1.0, 0.4, 0.0); // 橙
    ctf->AddRGBPoint(40.0,  1.0, 0.9, 0.0); // 亮黄

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(256);
    lut->SetRange(0, 50.0);

    for (int i = 0; i < 256; ++i) {
        double ratio = static_cast<double>(i) / 255.0;
        double val = 0 + ratio * 50.0;
        double rgb[3];
        ctf->GetColor(val, rgb);
        lut->SetTableValue(i, rgb[0], rgb[1], rgb[2], 1.0);
    }

    lut->Build();
    return lut;
}

vtkSmartPointer<vtkLookupTable> MainWindow::createHeightLUT() {
    vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();

    // 蓝 -> 浅蓝 -> 浅黄 -> 黄
    ctf->AddRGBPoint(0.0, 0.0, 0.2, 0.5);    // 深蓝
    ctf->AddRGBPoint(0.2, 0.0, 0.5, 0.9);    // 蓝

    // 0.2~0.5: 蓝 -> 浅绿
    ctf->AddRGBPoint(0.35, 0.2, 0.7, 0.4);   // 浅绿
    ctf->AddRGBPoint(0.5, 0.4, 0.8, 0.3);    // 明亮绿

    // 0.5~0.8: 浅绿 -> 浅黄
    ctf->AddRGBPoint(0.65, 0.8, 0.8, 0.2);   // 淡黄
    ctf->AddRGBPoint(0.8, 0.9, 0.9, 0.0);    // 黄

    // 0.8~1.0: 黄 -> 橙
    ctf->AddRGBPoint(0.9, 1.0, 0.85, 0.0);   // 浅橙
    ctf->AddRGBPoint(1.0, 1.0, 0.6, 0.0);    // 橙

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(256);
    lut->SetRange(0.0, 1);

    for (int i = 0; i < 256; ++i) {
        double rgb[3];
        ctf->GetColor(static_cast<double>(i)/255.0, rgb);
        lut->SetTableValue(i, rgb[0], rgb[1], rgb[2], 1.0);
    }
    lut->Build();
    return lut;
}

void MainWindow::on_loadpcap_clicked() {

    clearHighlightedPoints();

    if (cumulativeCloud) cumulativeCloud->clear();
    // 1. 强力清理：确保老数据和老状态完全消失
    if (m_processor->isRunning()) {
            m_processor->stop();
            // 稍微等待旧线程退出，防止竞态条件
            QElapsedTimer waitTimer;
            waitTimer.start();
            while(m_processor->isRunning() && waitTimer.elapsed() < 500) {
                QCoreApplication::processEvents(); // 保持界面不卡死
            }
        }



    if (playbackTimer->isActive()) playbackTimer->stop();

    if (pointCloudActor) {
            mainRenderer->RemoveActor(pointCloudActor);
            pointCloudActor = nullptr;
        }
    // 4. 重置全局指针，让它们不再指向即将销毁的内存
        satecloud = nullptr;
        currentFrameIndex = -1;

        // 5. 强制渲染器刷新一帧（现在是空的了）
        // 这步非常重要，它让渲染管线彻底“撒手”
        if (renderWindow) {
            renderWindow->Render();
        }

    // 如果你之前的异步还在跑，这里需要确保清理干净
    m_parser->clear();
    multiFrames.clear();
    frames8x8.clear();

    m_imageTimeMap.clear();

    // UI 清理逻辑
    clearHighlightedPoints();
    if (pointCloudActor) {
        mainRenderer->RemoveActor(pointCloudActor);
        pointCloudActor = nullptr;
    }

    // 2. 选择文件
    QString filename = QFileDialog::getOpenFileName(this, "加载", "", "PCAP (*.pcap)");
    if (filename.isEmpty()) return;

    QFileInfo pcapInfo(filename);
    QString videoDirPath = pcapInfo.absolutePath() + "/../Video";
    QDir videoDir(videoDirPath);

    if (videoDir.exists()) {
            QStringList files = videoDir.entryList({"*.jpg"}, QDir::Files, QDir::Name);
            for (const QString& fileName : files) {
                // 解析文件名：1773556274484_0879.jpg
                // 提取下划线前的部分
                QStringList parts = fileName.split('_');
                if (!parts.isEmpty()) {
                    qint64 ts = parts[0].toLongLong(); // 1773556274484
                    m_imageTimeMap.insert(ts, videoDir.absoluteFilePath(fileName));
                }
            }
        }

    // 3. 重置状态
    isRadar8x8Mode = true;
    ui->signal8_8->setChecked(true);
    isFirstFrame = true; // 确保第一帧逻辑能再次触发

    ui->frameSlider->setSingleStep(1);
    ui->frameSlider->setPageStep(1); // 这样点击轨道也只跳 1 帧
    // 启动总秒表（类成员变量）
    totalStopwatch.start();

    // 4. 异步解析
    // 注意：我们将耗时统计逻辑移入线程内部
    QtConcurrent::run([this, filename]() {
        QElapsedTimer innerParseTimer;
        innerParseTimer.start();

        bool ok = m_processor->run(filename); // 执行解析

        qint64 parseTime = innerParseTimer.elapsed();

        // // 解析彻底完成后的回调
        // QMetaObject::invokeMethod(this, [this, ok, filename, parseTime]() {
        //     if (ok) {
        //         frames8x8 = m_parser->getFrames();
        //         size_t total_frames = frames8x8.size();

        //         // 计算全局 MinMax (用于 Slider 初始化)
        //         // pcMinMax = calculate8T8MinMax(frames8x8);
        //         // initSlidersFromPointCloud(pcMinMax.get());

        //         // 找回你的性能日志
        //         qDebug() << "================ PCAP (Async Complete) ================";
        //         qDebug() << "File:" << QFileInfo(filename).fileName();
        //         qDebug() << "Frames:" << total_frames;
        //         qDebug() << "Parse Total Time:" << parseTime << "ms";
        //         qDebug() << "Avg Per Frame:" << (double)parseTime / total_frames << "ms";
        //         qDebug() << "Total Time (Click to End):" << totalStopwatch.elapsed() << "ms";
        //         qDebug() << "=======================================================";


        //     }
        // });
    });
}

// void MainWindow::onNewFrameReady(int index) {
//     // 必须用 const auto& 避免拷贝大数据
//     const auto& allFrames = m_parser->getFrames();
//     if (index < 0 || index >= allFrames.size()) return;

//     auto cloud = allFrames[index].Radar8T8Point.cloud;
//     if (cloud) {
//         // 关键：这是播放器用的容器
//         multiFrames.push_back(cloud);
//     }

//     if (index == 0) {
//         qint64 firstFrameMs = totalStopwatch.elapsed();

//         currentFrameIndex = 0;
//         satecloud = cloud;

//         QElapsedTimer renderTimer;
//             renderTimer.start();

//         displayPointCloud();

//         qint64 vtkTime = renderTimer.restart();

//         // if (!allFrames[0].Radar8T8Point.rdmap.empty()) {
//         //     showRDMap(rdMapPlot, allFrames[0].Radar8T8Point.rdmap.data());
//         // }
//         qint64 rdTime = renderTimer.elapsed();

//         qDebug() << "--- First Frame Render Detail ---";
//             qDebug() << "VTK Cloud Render:" << vtkTime << "ms";
//             qDebug() << "RD Map Plot:" << rdTime << "ms";

//         ui->frameSlider->setMinimum(0);

//     }

//     // 实时更新 Slider，允许解析过程中点击播放或拖动
//     ui->frameSlider->setMaximum(index);
// }

void MainWindow::onNewFrameReady(int index) {
    // 1. 安全获取底层槽位中的那一帧数据
    auto frame = m_parser->getFrameAt(index);

    // 2. 🌟 核心修正：无论是实时 PCAP 还是离线 BIN，只要来了，
    // 就必须第一时间精准同步到上层缓存中！放到 if-else 的外面！
    if (index >= frames8x8.size()) {
        frames8x8.resize(index + 1);
    }
    frames8x8[index] = frame;

    // 提取公共的车身动力学参数
    m_current8x8Speed = frame.egoSpeed;
    m_current8x8YawRate = frame.egoYawRate;

    // 3. 关键分流
    if (frame.start_time_ns != 0) {
        // ==========================================
        // 【网络/PCAP实时模式】
        // ==========================================
        qint64 currentPointTime = frame.start_time_ns + frame.timestamp_ns;
        updateCarDataDisplay(currentPointTime);

        ui->frameSlider->setMaximum(index);
        pcMinMax = calculate8T8MinMax(frames8x8);

        qint64 currentTotalMs = frame.timestamp_ns / 1000000;
        ui->horizontalSlider->setMaximum(static_cast<int>(currentTotalMs));

        // 第一帧逻辑
        if (index == 0) {
            qint64 absoluteTimeNs = frame.start_time_ns + frame.timestamp_ns;
            QString imgPath = findClosestImage(absoluteTimeNs);

            if (!imgPath.isEmpty()) {
                QPixmap pix(imgPath);
                if (!pix.isNull()) {
                    ui->videoLabel->setPixmap(pix.scaled(ui->videoLabel->size(),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }

            currentFrameIndex = 0;
            satecloud = frame.Radar8T8Point.cloud;

            QElapsedTimer renderTimer;
            renderTimer.start();
            displayPointCloud();

            qDebug() << "--- First Frame Rendered ---" << renderTimer.elapsed() << "ms";
            ui->frameSlider->setMinimum(0);
        }
    }
    else {
        // ==========================================
        // 【离线 BIN 模式】—— 保持安静，只打印，绝不干扰实时进度条
        // ==========================================
        m_validFrameIds.insert(index);
        qDebug() << "BIN loaded -> frameid:" << index << " ok " << index << "";
    }
}
QString MainWindow::findClosestImage(qint64 absoluteTimeNs) {
    if (m_imageTimeMap.isEmpty()) return "";

    qint64 targetMs = absoluteTimeNs / 1000000; // 纳秒转毫秒

    // 在 QMap 中通过二分查找找到第一个不小于 targetMs 的迭代器
    auto it = m_imageTimeMap.lowerBound(targetMs);

    // 情况 A：目标时间比所有图片都晚
    if (it == m_imageTimeMap.end()) return (it - 1).value();

    // 情况 B：目标时间比所有图片都早
    if (it == m_imageTimeMap.begin()) return it.value();

    // 情况 C：目标时间在中间，找最接近的一个
    auto prevIt = it - 1;
    if (qAbs(it.key() - targetMs) < qAbs(prevIt.key() - targetMs)) {
        return it.value();
    }
    return prevIt.value();
}

// void MainWindow::onRealtimeCloudReady(pcl::PointCloud<PointXYZRGBWithProperties>::Ptr cloud, uint32_t frameId) {
//     // 埋点 T3
//         LatencyMonitor::instance().record(frameId, "T3_UI_Recv");

//     qDebug() << ">>>>>> UI RECEIVE SIGNAL! <<<<<<" << frameId; // 检查这一行输出吗？
//     if (!cloud || cloud->empty()) return;

//     // --- A. 模拟离线加载时的初始化状态 ---
//     if (isFirstRealtimeFrame) {
//         isFirstFrame = true; // 触发 displayPointCloud 内部的 pointCloudActor 初始化
//         isFirstRealtimeFrame = false;
//         totalStopwatch.restart();

//         // 如果你有累积逻辑，先清空之前的
//         if (cumulativeCloud) cumulativeCloud->clear();
//         frameBuffer.clear();
//     }

//     // --- B. 适配你的 displayPointCloud 变量 ---
//     // 你的渲染函数依赖 satecloud 变量，直接把实时收到的云赋值给它
//     satecloud = cloud;

//     // --- C. 调用你现有的渲染逻辑 ---
//     // 这个函数会自动执行：过滤(filter) -> 积累(Accumulate) -> 着色(Colorize) -> VTK更新
//     displayPointCloud();

//     // 埋点 T4
//     LatencyMonitor::instance().record(frameId, "T4_Rendered");

//     // 汇总报告
//     LatencyMonitor::instance().report(frameId);

//     static LatencyMonitorWidget *monitor = new LatencyMonitorWidget();
//         monitor->show();
//         monitor->raise(); // 确保在最前面
//     // --- D. UI 反馈 ---
//     // ui->label_frame_id->setText(QString("Live ID: %1").arg(frameId));
//     // ui->label_pts->setText(QString("Pts: %1").arg(cloud->size()));
// }

// void MainWindow::on_udpOnline_clicked()
// {
//     isFirstFrame = true; // 关键：让 displayPointCloud 走初始化路径
//         isFirstRealtimeFrame = true;
//         m_radarMgr->start("\\Device\\NPF_{FC1EC71A-2DAD-4FDE-9FB5-7D649D3651AF}", "", 7001);
// }

void MainWindow::on_udpOnline_clicked()
{
    if (!onlineDlg) {
        onlineDlg = new RadarOnlineDialog(this);

        connect(onlineDlg, &QObject::destroyed, [this](){
                    onlineDlg = nullptr;
                });
        // 当对话框点 Start 时，执行真正的启动逻辑
        connect(onlineDlg, &RadarOnlineDialog::startCapture,
                        this, &MainWindow::handleStartCapture);

                // ✅ 停止捕获也统一指向 handleStopCapture
        connect(onlineDlg, &RadarOnlineDialog::stopCapture,
                        this, &MainWindow::handleStopCapture);

        connect(onlineDlg, &RadarOnlineDialog::requestRecording,
                        this, &MainWindow::handleRecordingRequest);

        connect(onlineDlg, &RadarOnlineDialog::requestMirror,
                        m_surface, &VideoSurface::setMirrored);

    }
    onlineDlg->show();
    onlineDlg->raise();
}

void MainWindow::onRealtimeCloudReady(pcl::PointCloud<PointXYZRGBWithProperties>::Ptr cloud, uint32_t frameId,float speed, float yawRate) {
    // 埋点 T3
    LatencyMonitor::instance().record(frameId, "T3_UI_Recv");

    if (!cloud || cloud->empty()) return;

    isRadar8x8Mode=true;
    m_current8x8Speed = speed;
    m_current8x8YawRate = yawRate;
    updateCarDataDisplay(0);
    // A & B 逻辑保持不变...
    if (isFirstRealtimeFrame) {
            isFirstFrame = true;
            isFirstRealtimeFrame = false;

            // --- 1. PCL 数据清理 ---
            frameBuffer.clear();
            lastAccumulatedCloud = nullptr;
            if (cumulativeCloud) cumulativeCloud->clear();
            if (filteredCloudPtr) filteredCloudPtr->clear();

            // --- 2. VTK 数据清理（非常重要） ---
            // 强制清空 VTK 对象内部的顶点和单元，防止残留
            if (pointCloudPolyData) {
                pointCloudPolyData->Initialize();
            }

            // --- 3. 强制移除旧的 Actor ---
            // 有时候多次初始化会导致 Renderer 里堆积了多个 Actor
            if (pointCloudActor) {
                mainRenderer->RemoveActor(pointCloudActor);
                pointCloudActor = nullptr; // 这样 displayPointCloud 会走 isFirstFrame 初始化路径
            }

            totalStopwatch.restart();
        }
    satecloud = cloud;

    // C. 渲染逻辑
    displayPointCloud();

    // 埋点 T4 & 汇总
    LatencyMonitor::instance().record(frameId, "T4_Rendered");
    LatencyMonitor::instance().report(frameId);

    // 💡 注意：这里不需要再 new LatencyMonitorWidget 了！
    // 因为它已经作为子部件嵌入在 RadarOnlineDialog 里面了，
    // 单例模式会自动把数据更新到对话框里的那个面板上。
}


void MainWindow::handleStopCapture() {
    // 停止雷达
    m_radarMgr->stop();

    // 停止摄像头
    if (m_camera && m_camera->status() == QCamera::ActiveStatus) {
        m_camera->stop();
    }
}

void MainWindow::handleStartCapture(QString device, QString ip, int port, QString cameraName, QSize res, double fps, int rotation) {
    // 0. 停止之前的播放器
    isFirstFrame = true;
    isFirstRealtimeFrame = true;
    if (m_player) m_player->stop();

    // 1. 启动雷达采集
    m_radarMgr->start(device, ip, port);

    // 2. 启动摄像头
    if (m_camera) {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }

    // 根据选中的 ID 创建摄像头
    m_camera = new QCamera(cameraName.toUtf8(), this);

    // 🚀 --- 设置核心参数 (分辨率 & 帧率) ---
    // 必须在 start() 之前设置 viewfinderSettings
    QCameraViewfinderSettings settings;
    settings.setResolution(res);
    settings.setMinimumFrameRate(fps);
    settings.setMaximumFrameRate(fps);

    // 如果你知道摄像头的像素格式需求（可选），也可以在这里设置
    // settings.setPixelFormat(QVideoFrame::Format_YUYV);


    m_camera->setViewfinderSettings(settings);

    // 🚀 --- 设置旋转 ---
    // 在 Qt 5 中，QCamera 不直接负责 UI 层的旋转。
    // 我们需要把 rotation 传给渲染表面 (m_surface)
    if (m_surface) {
        // 假设你的 VideoSurface 类有一个 setRotation 方法
        m_surface->setRotation(rotation);
        if (onlineDlg) {
                     m_surface->setMirrored(onlineDlg->isMirrorChecked());
                }

        m_camera->setViewfinder(m_surface);
    }

    // 启动摄像头
    m_camera->start();

    qDebug() << "Radar started on:" << device;
    qDebug() << "Camera Config: " << res.width() << "x" << res.height() << "@" << fps << "fps, Rotate:" << rotation;
}

void MainWindow::handleRecordingRequest(bool start) {
    if (start) {
        // 1. 生成基于时间戳的文件夹
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString saveDir = QCoreApplication::applicationDirPath() + "/Recordings/" + timestamp;

        QDir().mkpath(saveDir + "/Video");
        QDir().mkpath(saveDir + "/Radar");

        // 2. 开启视频异步录制 (分发路径)
        if (m_surface) {
            m_surface->setRecording(true, saveDir + "/Video");
        }

        // 3. 记录当前路径，供雷达使用
        QString pcapFilePath = saveDir + "/Radar/radar_raw.pcap";
                if (m_radarMgr) {
                    m_radarMgr->setRecording(true, pcapFilePath);
                }
        m_isRecording = true;


        qDebug() << "Sync Recording Started: " << saveDir;
    } else {
        // 停止视频录制
        if (m_surface) m_surface->setRecording(false);

        if (m_radarMgr) {
                    m_radarMgr->setRecording(false);
                }

        m_isRecording = false;
        m_currentRecordDir = "";
        qDebug() << "Sync Recording Stopped.";
    }
}

int MainWindow::findClosestRadarFrame(qint64 relativeTimeNs) {
    if (frames8x8.empty()) return -1;

    int low = 0;
    int high = frames8x8.size() - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (frames8x8[mid].timestamp_ns < relativeTimeNs)
            low = mid + 1;
        else
            high = mid - 1;
    }

    // 检查边界并返回最接近的一个
    int index = qBound(0, low, (int)frames8x8.size() - 1);
    return index;
}

void MainWindow::on_load16TrBin_clicked()
{

    QString folderPath = QFileDialog::getExistingDirectory(this, "选择包含不连续帧号的 BIN 文件夹");
        if (folderPath.isEmpty()) return;

        m_currentBinFolderPath = folderPath;

        // 1. 雷达容器与 3D 舞台深度清理
        m_parser->clear();
        frames8x8.clear();
        clearHighlightedPoints();
        if (pointCloudActor) {
            mainRenderer->RemoveActor(pointCloudActor);
            pointCloudActor = nullptr;
        }

        // 2. 维持模式激活标记
        isRadar8x8Mode = true;
        isFirstFrame = true;

        // 🌟 注意：此时不要去动 ui->frameSlider->setMaximum。
        // 因为这套模式以 AVI 视频为主体，它的进度条最大值应由你加载的 AVI 视频文件总帧数决定的！

        qDebug() << "MainWindow: load bin...";

        // 3. 跨线程异步安全投递
        QMetaObject::invokeMethod(m_processor, "processBinFolder",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, folderPath));
}

void MainWindow::onBinFolderLoadingFinished() {
    if (!isRadar8x8Mode) return;
    frames8x8 = m_parser->getFrames();

    if (m_validFrameIds.empty()) {
        qDebug() << "Errro No bin file";
        return;
    }

    QDir dir(m_currentBinFolderPath);
        if (dir.exists()) {
            QStringList videoFilters;
            videoFilters << "*.avi";

            // 1. 先在当前选中的 BIN 文件夹内部搜
            QStringList videoFiles = dir.entryList(videoFilters, QDir::Files);

            // 2. 如果当前目录下没找到，自动往上一级（同级目录）去捞
            if (videoFiles.isEmpty()) {
                QDir parentDir = dir;
                if (parentDir.cdUp()) {
                    videoFiles = parentDir.entryList(videoFilters, QDir::Files);
                    if (!videoFiles.isEmpty()) {
                        dir = parentDir; // 切换到父目录路径
                    }
                }
            }

            // 3. 如果找到了 AVI 视频，直接静默加载
            if (!videoFiles.isEmpty()) {
                QString aviPath = dir.absoluteFilePath(videoFiles.first());
                qDebug() << "Video path:" << aviPath;

                if (m_player) {
                    m_player->stop();

                    // 🌟 Qt 5 标准语法：通过 QMediaContent 包装本地文件 URL
                    m_player->setMedia(QMediaContent(QUrl::fromLocalFile(aviPath)));

                    // 1. 计算雷达第一帧对应的视频毫秒数
                                    int absoluteMinFrame = *m_validFrameIds.begin(); // 例如 13
                                    qint64 targetVideoPosMs = static_cast<qint64>(absoluteMinFrame) * 50; // 50ms一帧

                                    // 2. 强行启动播放（逼迫解码器吐出数据刷新 UI）
                                    m_player->play();

                                    // 3. 瞬间移位并刹车暂停！
                                    // 这样既刷新了画面，又把视频定格在了雷达首帧的位置，按钮依然显示 "Play"
                                    m_player->setPosition(targetVideoPosMs);
                                    m_player->pause();

                    // 重置播放控制按钮文本
                    ui->playPauseButton->setText("Play");
                    qDebug() << "Vedio ready";
                }
            } else {
                qDebug() << "cann't find avi file。";
            }
        }


    // 🌟 核心改动：从 set 里直接拿第一个（最小）和最后一个（最大）硬件帧号
    int absoluteMinFrame = *m_validFrameIds.begin();  // 比如 13
    int absoluteMaxFrame = *m_validFrameIds.rbegin(); // 比如 180

    // 严丝合缝地卡死进度条范围
    ui->frameSlider->setMinimum(absoluteMinFrame);
    ui->frameSlider->setMaximum(absoluteMaxFrame);

    // 让当前播放指针直接空降到“有数据的起点”
    currentFrameIndex = absoluteMinFrame;
    ui->frameSlider->setValue(currentFrameIndex);

    // 现场曝光第一帧有效画面
    auto &frame = frames8x8[currentFrameIndex];
    satecloud = frame.Radar8T8Point.cloud;
    displayPointCloud();

    if (!frame.Radar8T8Point.rdmap.empty()) {
        showRDMap(rdMapPlot, frame.Radar8T8Point.rdmap.data());
    }

    qDebug() << "frame index: [" << absoluteMinFrame << " - " << absoluteMaxFrame << "]";
}
