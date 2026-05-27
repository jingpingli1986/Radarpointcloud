#ifndef RSPPROCESS_H
#define RSPPROCESS_H
// #include <demoMex/dspProssing.h>
// #include <dspProssing.h>
#include <QDebug>
#include <QLibrary>
#include <QImage>
#include <QColor>
#include <qcustomplot.h>
#include <cmath>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QSurfaceDataArray>
#include <QtDataVisualization/QValue3DAxis>
#include <limits>
#include <QLinearGradient>
#include <QtDataVisualization/Q3DInputHandler>
#include <iostream>
#include <fstream>
#include <string>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <memory>
#include <cmath>
#include <vector>
#include <customradarPoint.h>
#include "mdf2bin.h"
#include <iomanip>
#include "RadarBinAnalysis.h"

#include "RspProcessor.h"


using namespace QtDataVisualization;

// typedef void (*RspProssingFunc)(float*, VehiclesInfo_t*, RadarInfo_t*, PointCloud2_t*, float*);

struct PointCloud8T8Data
{
    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cloud;  // 点云
    std::vector<float> rdmap;                                           // RD map
};
struct pointandtimedata8x8
{
    size_t indexno;
    uint64_t start_time_ns;
    uint64_t timestamp_ns;

    float egoSpeed = 0.0f;     // 新增：车速
    float egoYawRate = 0.0f;   // 新增：角速率

    PointCloud8T8Data Radar8T8Point;   // 8×8 点云 + RD map
};

QImage dspdll_Test();
QImage createRangeDopplerImage(float* rdmap);
QColor jetColormap(double normVal);
void setupRangeDopplerPlot(QCustomPlot *customPlot,const QString& filename);
void setupRangeDopplerPlot3D(Q3DSurface *graph);
float* readBinToFloatPtr(const QString& filename);
std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> setupRangeDopplerPlotnew(QCustomPlot *customPlot,const QString& filename);
void printPointCloud2(const PointCloud2_t* ptPointCloud);
bool writePointCloudToCsv(const PointCloud2_t& data, const std::string& filename);
std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> convertPointCloud2ToPCL(const PointCloud2_t& sourceData,int currentFrameIndex);
float* uint16VectorToFloatPtr(const std::vector<uint16_t>& inputVector);
float* int16VectorToFloatPtr(const std::vector<int16_t>& inputVector);
std::vector<pointandtimedata8x8> GetRadar8T8Datafrommf4(const QString& filename,const QString& savePath,double start,double end);
std::vector<pointandtimedata8x8> GetRadar8T8Datafrommf4byfiles(const QString& filename,const QString& savePath,double start,double end);


bool writePointCloudCsvHeader(const std::string& filename);
bool writePointCloudToCsv_Append(int frameIndex, double timestamp,
                                 const PointCloud2_t& data,
                                 const std::string& filename);
void showRDMap(QCustomPlot *customPlot, const float* rdmap, int fftSize = FFT_2D_SIZE, int rngNum = RNG_NUM);

void Save8T8VectorToBin(const std::string& filename,const std::vector<pointandtimedata8x8>& data);
void Init8T8BinFile(const std::string& filename);
void Save8T8FrameToBin(const std::string& filename,
                       const pointandtimedata8x8& frame);
void Finalize8T8BinFile(const std::string& filename, uint64_t finalFrameCount);
std::vector<pointandtimedata8x8> Load8T8VectorFromBin(const std::string& filename) ;
bool LoadSingle8T8Frame(std::ifstream& ifs, pointandtimedata8x8& data);
void updateRangeDopplerPlot(QCustomPlot *customPlot, const float *rdmap);


std::wstring utf8_to_wstring(const std::string& str);
#endif // RSPPROCESS_H
