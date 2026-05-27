#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mdf2bin.h"
#include <vtkOutlineSource.h>
#include <pcl/io/pcd_io.h>
#include <QMouseEvent>
#include <QMainWindow>
#include <QTimer>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/make_shared.hpp>
#include <vtkGenericOpenGLRenderWindow.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <QVector3D>
#include <vtkFrustumSource.h>
#include <pcl/visualization/point_picking_event.h>
#include <vtkAreaPicker.h>
#include <vtkSmartPointer.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <fstream>
#include <vtkPointPicker.h>
#include <vtkIdFilter.h>
#include <vtkExtractSelectedFrustum.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>
#include <vtkExtractSelection.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkCellPicker.h>
#include <vtkPlaneSource.h>
#include <vtkRenderer.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkRendererCollection.h>
#include <vtkBillboardTextActor3D.h>
#include <QMessageBox>
#include <pcl/kdtree/kdtree_flann.h>
#include <QDebug>
#include <QFileDialog>
#include <vtkRenderWindow.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkExtractGeometry.h>
#include <vtkPlanes.h>
#include <vtkGeometryFilter.h>
#include "radar_simulator.h"
#include "pointcloud_loader.h"
#include "customradarPoint.h"
#include "RadarBinAnalysis.h"
#include <vtkLine.h>
#include <vtkCamera.h>
#include <vtkTransform.h>
#include <vtkTextActor3D.h>
// #include "videoclass.h"
#include <vtkPointLocator.h>
#include "FrameRangeDialog.h"
#include "MdfExtractDialog.h"
#include "QxtSpanSlider.h"
#include "filterpointdata.h"

#include <QDockWidget>
#include <QSettings>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QToolBar>
#include <vtkInteractorStyleUser.h>
#include <vtkRenderWindowInteractor.h>
#include <pointcloudcolorizer.h>
#include <rspprocess.h>
#include <qcustomplot.h>
#include <vtkCubeAxesActor2D.h>
#include "vtkStringArray.h"
#include "vtkPolyLine.h"
#include "vtkAxisActor2D.h"
#include "LoadedFilesDock.h"
#include <vtkPointGaussianMapper.h>
#include <vtkLightCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkColorTransferFunction.h>
#include <pcap_module/pcapprocessor.h>
#include "pcap_module/radardataparser.h"
#include "RadarCaptureManager.h"
#include "LatencyMonitor.h"

#include "LatencyMonitorWidget.h"
#include "RadarOnlineDialog.h"
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>

class VideoSurface;   // 前置声明
class QMediaPlayer;   // 前置声明

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;


}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void OnInteractionCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onBinFolderLoadingFinished() ;
    void handleRecordingRequest(bool start);
    void handleStartCapture(QString device, QString ip, int port, QString cameraName, QSize res, double fps, int rotation);
    void handleStopCapture();
    void onNewFrameReady(int index);
    void onXYAzimuthChanged(int value);
    void delayedGridInitialization();

    //勾选point cloud viewer and video viewer
    void onDisplayPointcloudChanged(int state);
    void onDisplayVideoChanged(int state);

    void SatalitePointCloud();
    void updatePointCloud();
    void on_saveButton_clicked();  // 保存按钮槽函数
    void on_loadButton_clicked();  // 加载一帧数据
    void on_saveMultiFrameButton_clicked(); //保存多帧点云数据
    void on_loadMultiFrameButton_clicked();  //加载多帧点云数据

    void on_playPauseButton_clicked();
    //void on_frameSlider_valueChanged(int value);
    // 添加槽函数或普通成员函数声明


    void on_checkBoxSelectMode_toggled(bool checked);
    void on_selectedPointChanged(int index);


    void on_readRadar4T4Bin_clicked();

    void on_LoadVideo_clicked();

    void on_TransferMDF4toPointdata_clicked();

    void on_pushButton_clicked();

    void spanSliderValueChanged(int);          // Slider 改变值时调用
    void spanLineEditChanged(const QString &); // LineEdit 改变文本时调用

    void onResetFilterClicked();
    void onApplyFilterClicked();
    void setUserXRange(int value);
    void setUserYRange(int value);
    void setSectorStartAngle(int value);
    void setSectorEndAngle(int value);
    void onColorModeChanged(int index);
    void setMaxFramesToAccumulate(int count);
    void on_ViewupcomboBox_currentIndexChanged(int index);

    void on_XYAziInitial_clicked();



    void on_XYZ45_clicked();

    void on_prevFrameButton_clicked();

    void on_nextFrameButton_clicked();


    void on_loadpcap_clicked();
    void onRealtimeCloudReady(pcl::PointCloud<PointXYZRGBWithProperties>::Ptr cloud, uint32_t frameId,float speed,float yawRate);

    void on_udpOnline_clicked();

    void on_load16TrBin_clicked();

private:
    void setCameraCustomAngle(const double position[3],
                                          const double focalPoint[3],
                                          const double desiredViewUp[3]);
    void computeAzElRollFromCamera(
        const double position[3],
        const double focalPoint[3],
        const double viewUp[3],
        double &azimuthDeg,
        double &elevationDeg,
        double &rollDeg);

    void MainWindow::setCameraByAzElRollDistance(
        double azimuthDeg,
        double elevationDeg,
        double distance,
        double rollDeg,
        const double focalPoint[3]);
    void MainWindow::setCameraByAzElRollDistanceDirect(
        double azimuthDeg,
        double elevationDeg,
        double distance,
        double rollDeg,
        const double focalPoint[3]);
    void createAxis2D();
    bool isRadar8x8Mode = false;

    std::vector<pointandtimedata8x8> frames8x8;


    vtkSmartPointer<vtkActor> xAxisActor;
    vtkSmartPointer<vtkActor> yAxisActor;
    vtkSmartPointer<vtkActor> tickActor;
    // 🌟 刻度线数据 (新增成员变量) 🌟
    vtkSmartPointer<vtkPoints> tickPoints;
    vtkSmartPointer<vtkCellArray> tickLines;

    vtkSmartPointer<vtkCubeAxesActor2D> cubeAxesActor;
    bool colorUpdateOnly = false;

    pointcloudcolorizer colorizer;
    ColorMode currentColorMode = ColorMode::RadialVelocity;

    QCustomPlot *rdMapPlot;

    double userXRange = 150.0;
    double userYRange = 150.0;
    double sectorStartAngle = 30.0;
    double sectorEndAngle = 150.0;
    vtkSmartPointer<vtkActor> sectorActor = nullptr;
    vtkSmartPointer<vtkActor> sectorBgActor = nullptr;
    void createSectorGeometry();
    void updateSectorGeometry();

    // 用于存储所有标签 Actor 的列表
    QList<vtkSmartPointer<vtkFollower>> labelFollowers;
    vtkSmartPointer<vtkCellArray> gridLines;
    vtkSmartPointer<vtkPoints> gridPoints;
    double currentGridSpacing = 25.0;
    vtkSmartPointer<vtkPolyData> gridPolyData;

    void AddGridAndLabelsDynamically();
    double findNiceNumberSpacing(double idealSpacing);
    // VTK 交互事件槽
    void onVtkInteraction();
    double dynamicXOffset = 0.0;

    void saveSettings();
    void loadSettings();
    void testDockTabify();
    void setupApplicationSettings();
    Ui::MainWindow *ui;
    //勾选point cloud viewer and video viewer
    void updateLayout();

    //vtkRenderer* renderer;
    vtkSmartPointer<vtkRenderer> mainRenderer;
    vtkSmartPointer<vtkAxesActor> dynamicAxesActor;

    vtkSmartPointer<vtkActor> pointCloudActor;
    vtkSmartPointer<vtkPolyData> pointCloudPolyData;
    vtkSmartPointer<vtkPolyDataMapper> pointCloudMapper;
    // vtkSmartPointer<vtkPointGaussianMapper> pointCloudMapper;
    vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter;
    // 多帧显示buffer
    QList<std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>> frameBuffer;
    int maxFramesToAccumulate = 1;
    // 追踪最后一次被累积到 frameBuffer 的点云指针
    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> lastAccumulatedCloud = nullptr;
    bool isProgrammaticallyChangingSlider = false;
    void setupBackgroud();

    void setupSpanSliders();

    void setupCameraWithOriginAtViewportBottom10Percent();
    void AddYlabel(double sizeX,double sizeY);
    void AddSector(double centerX, double centerY, double centerZ,
                   double radius, double startAngleDeg, double endAngleDeg, int resolution);
    vtkSmartPointer<vtkActor> createGrid(double spacing, int count );
    vtkSmartPointer<vtkTextActor> createTextLabel(const std::string& text, double x, double y, double z);
    //初始化点云显示
    void setupVTKWidget();
    QTimer *timer;

    // 点云数据
    //boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> cloud;

    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> satecloud;
    vtkSmartPointer<vtkActor> gridActor;



    bool isFirstFrame = true;

    void displayPointCloud();

    RadarSimulator simulator;
    Customcradar customdata;
    std::vector<std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>> multiFrames;
    // 保存多帧到BIN
    void saveMultiFrameBin(const std::string& filename,
                           int startFrame,
                           int endFrame,
                           const std::vector<pointandtimedata>& frames);

    // 加载多帧BIN文件
    void loadMultiFrameBin(const QString& filename);

    // 清理多帧缓存
    void clearMultiFrames();
    int currentFrameIndex = 0;
    std::vector<pointandtimedata> frames;
    QTimer* playbackTimer;
    bool isPlaying = false;
    /*
    bool findNearestPoint(double x, double y, double z, pcl::PointXYZRGB &outPoint);
*/
    vtkSmartPointer<vtkPointPicker> pointPicker;
    bool eventFilter(QObject* obj, QEvent* event);

    vtkSmartPointer<vtkActor> selectedPointActor;

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;

    vtkSmartPointer<vtkRenderer> selectedRenderer;
    vtkSmartPointer<vtkActor> selectedActor;
    vtkSmartPointer<vtkAreaPicker> areaPicker;

    int pressX = 0;
    int pressY = 0;

    void doAreaPick(int x0, int y0, int x1, int y1);

    bool selectionModeEnabled = false;
    vtkSmartPointer<vtkActor> existingPlaneActor;

    template <typename CloudT>
    void handlePickedPoint(const CloudT& cloud, int pickedPointId);

    template <typename PointT>
    QString getPointInfo(const PointT& pt, int id);
    vtkSmartPointer<vtkCellPicker> cellPicker ;

    vtkSmartPointer<vtkPoints> highlightedPoints;
    vtkSmartPointer<vtkPolyData> highlightedPolyData;
    vtkSmartPointer<vtkPolyDataMapper> highlightedMapper;
    vtkSmartPointer<vtkActor> highlightedActor;
    void addPointIdToComboBox(int pickedPointId);
    void removePointFromHighlighted(int pickedPointId) ;
    void removePointFromComboBox(int pickedPointId);
    QSet<int> selectedPointIds;

    QMediaPlayer* m_player = nullptr;
    VideoSurface* m_surface = nullptr;
    void addPointToSelection(int pointId);
    void MainWindow::clearHighlightedPoints();

    std::vector<TimestampedCarData> CarData;
    void updateCarDataDisplay(uint64_t PointTime);
    std::vector<TimestampedCarSRSYawRATE> CarSRSYawData;
    QString formatTime(qint64 ms);
    void updateInfoPanelPosition();

    void initSlidersFromPointCloud(const PointCloudDataminMax* data);
    QMap<QxtSpanSlider*, SpanSliderMapping> sliderMappings;
    PointCloudDataminMax *pointminmaxData;
    FilterCriteria currentFilter;
    bool filterAppliedFlag =false;
    std::unique_ptr<PointCloudDataminMax> MainWindow::calculateMultiFrameMinMax(
        const std::vector<std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>>& multiFrames);
    std::unique_ptr<PointCloudDataminMax> pcMinMax;


    std::unique_ptr<PointCloudDataminMax> MainWindow::calculate8T8MinMax(const std::vector<pointandtimedata8x8>& frames8x8);

    void handlePickedPointnew(vtkIdType pickedPointId);
    QSet<QPair<int, int>> selectedPointKeys;
    void addPointToSelectionnew(int frameIndex, int originalPointId);
    void removeSelectionByKey(const QPair<int, int>& selectionKey);
    template <typename PointT>
    QString getPointInfonew(const PointT& pt, const QPair<int, int>& selectionKey);
    void addPointIdToComboBoxnew(const QPair<int, int>& selectionKey);
    void MainWindow::doAreaPicknew(int x0, int y0, int x1, int y1);

    vtkSmartPointer<vtkPoints> originPoints;
    vtkSmartPointer<vtkCellArray> originLines;
    vtkSmartPointer<vtkPolyData> originPolyData;
    vtkSmartPointer<vtkActor> originActor;
    vtkSmartPointer<vtkTransform> planeTransform;
    vtkSmartPointer<vtkCamera> baseCamera;


    bool isUpdatingFromVideo = false;
    bool isUpdatingFromCloud = false;
    void updatePointCloudFromVideo(qint64 videoMs);

    LoadedFilesDock* loadedFilesDock;

    void updateColorBar(const QString& arrayName, vtkLookupTable* lut);
    vtkSmartPointer<vtkScalarBarActor> scalarBarActor;
    vtkSmartPointer<vtkLookupTable> createRadarLUT() ;
    vtkSmartPointer<vtkLookupTable> createRadarSNRLUT() ;
    vtkSmartPointer<vtkLookupTable> createHeightLUT();


    // 1. 存储点云的逻辑边界 [minX, maxX, minY, maxY, minZ, maxZ]
    double currentCloudBounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    // 2. 存储中心点坐标
    double cloudCenterX = 0.0;
    double cloudCenterY = 0.0;

    // 3. 存储建议的相机缩放比例
    double suggestedParallelScale = 10.0;
    void calculateCloudMetrics(std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cloud);



    void initInfoPanelStyle();

    PcapProcessor* m_processor; // 处理 PCAP 文件的读取和组包
    RadarDataParser* m_parser;   // 解析组包后的原始数据并存储到 m_frames
    QElapsedTimer totalStopwatch;

    // --- 性能优化相关的持久化对象 ---
    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> filteredCloudPtr;
    vtkSmartPointer<vtkLookupTable> radarLUT; // 预创建的色带
    vtkSmartPointer<vtkLookupTable> heightLUT;

    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cumulativeCloud;

    RadarCaptureManager* m_radarMgr; // 你的实时采集模块
    bool isFirstRealtimeFrame = true; // 类似你离线的 isFirstFrame
    RadarOnlineDialog *onlineDlg = nullptr;

    QCamera *m_camera = nullptr;

    // 录制状态
    bool m_isRecording = false;
    QString m_currentRecordDir;

    // 状态更新定时器
    QTimer* m_statusTimer = nullptr;

    QMap<qint64, QString> m_imageTimeMap;
    QString findClosestImage(qint64 absoluteTimeNs);

    int findClosestRadarFrame(qint64 relativeTimeNs);

    float m_current8x8Speed = -999.0f;
    float m_current8x8YawRate = -999.0f;
    std::set<int> m_validFrameIds;//16*16bin
    QString  m_currentBinFolderPath;//16*16 avi path

};






#endif // MAINWINDOW_H
