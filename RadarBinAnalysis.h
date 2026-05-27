#ifndef RADARBINANALYSIS_H
#define RADARBINANALYSIS_H

#include <iostream>
#include <fstream>
#include "Radar_4T4RMain_Dll.h"
#include "mclmcrrt.h"
#include "mclcppclass.h"
#include <QDebug>
#include <iomanip>
#include <QFileDialog>
#include "mdf2bin.h"


const int DIM1 = 768;
const int DIM2 = 1024;
const int DIM3 = 4;


struct PointData {
    std::vector<mwString> strArray;
    std::vector<std::vector<double>> dblArray;
};

struct pointandtimedata{
    size_t indexno;
    uint64_t start_time_ns;
    uint64_t timestamp_ns;
    PointData Radar4T4Point;
};
std::vector<double> mwArrayToDoubleArray(const mwArray& fieldValue) ;
void GetRadar4T4DatafromBIN(const QString& filename,const QString& savePath);

std::string mwStringToStdString(const mwString& mwStr);



void prependPath(const QString& newPath) ;

void saveStrArrayToCSV(const PointData& pointData, const std::string& filename) ;
void saveDblArrayToCSV(const PointData& pointData, const std::string& filename);
void saveFieldDataToCSV(const PointData& pointData, const std::string& filename);
void saveFieldDataToCSV(const PointData& pointData, const std::string& filename,std::string timestring,size_t number);
std::vector<uint16_t> reorderTX(const std::vector<uint16_t>& input,
                                int txCount,
                                int blockSize) ;

void processPixelsMatlabOrder(const std::vector<uint16_t> &pixels,
                              int samplenum, int chirpnum,
                              std::vector<uint16_t> &data1D);

void GetRadar4T4Datafrommf4(const QString& filename,const QString& savePath,double start,double end);
void GetRadar4T4Datafrommf4(const QString& filename, const QString& savePath, double start, double end, const std::vector<TimestampedCarData>& CarData, const std::vector<TimestampedCarSRSYawRATE> &CarSRSYawData);
void savePointsToBin(const std::string& filename,const std::vector<pointandtimedata>& data);
std::vector<pointandtimedata> loadPointsFromBin(const std::string& filename) ;

double GetwheSpeed(const std::vector<TimestampedCarData>& CarData,const uint64_t PointTime);
double GetYawData(const std::vector<TimestampedCarSRSYawRATE>& CarSRSYawData,const uint64_t PointTime);
#endif // RADARBINANALYSIS_H
