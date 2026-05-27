#ifndef MDF2BIN_H
#define MDF2BIN_H
#include <QDebug>
#include <QApplication>
#include <mdf/mdffactory.h>
#include <mdf/mdfreader.h>
#include <iostream>
#include "mdf/mdffile.h"
#include <memory>
#include <mdf/idatagroup.h>
#include <mdf/ichannelgroup.h>
#include <mdf/ichannel.h>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <QProgressDialog>
#include <filesystem>   // C++17 用于路径操作
#include <sstream>      // 用于字符串流
#include <mdf/ichannel.h>

namespace fs = std::filesystem;


//using namespace std;

struct FrameMetadata {
    uint64_t file_start_time_ns;        // MDF文件本身的起始时间
    uint64_t relative_timestamp_ns;     // 相对于文件起始时间的时间戳（纳秒）
    size_t frame_index;                 // 帧的全局索引
    std::string bin_filepath;           // 存储该帧数据的 .bin 文件的完整路径
};


struct TimestampedData {
    uint64_t start_time_ns;
    uint64_t timestamp_ns;
    std::vector<uint8_t> data;
    size_t number;
};


struct TimestampedCarSRSYawRATE {
    uint64_t start_time_ns;
    uint64_t timestamp_ns;
    double data;
    size_t number;
};

struct TimestampedCarData {
    uint64_t start_time_ns;
    uint64_t timestamp_ns;
    double data;
    size_t number;
};

std::vector<uint16_t> decodeRaw12(const std::vector<uint8_t>& data) ;

std::vector<int16_t> process_matlab_typecast_and_bitshift(    const std::vector<uint16_t>& pixels,    int shift_amount = 4);
std::vector<uint8_t> extractRaw12FromFrame(const std::vector<uint8_t>& frame_bytes) ;

std::vector<uint8_t> extractRaw12Frames(const std::vector<uint8_t>& frame_bytes, size_t offset, size_t step);

std::string convertTimestamp(uint64_t nanoseconds);

//转换 all mdf 到一个BIN 文件
std::vector<TimestampedData> readallmdftobin(std::string filename);
std::vector<TimestampedData> readallmdftobin(std::string filename,double start,double end);

std::vector<TimestampedCarData> readallmf4CarData(std::string filename);
std::vector<TimestampedCarData> readallmf4CarData8T8(std::string filename);
std::vector<TimestampedCarSRSYawRATE> readallmf4CarSRSYawRate(std::string filename);
std::vector<TimestampedCarSRSYawRATE> readallmf4CarSRSYawRate8T8(std::string filename);
bool readBinFile(const std::string& filename, std::vector<uint8_t>& frame_bytes) ;

void TransferOneFrame2RadarADC();

// 8*8
std::vector<uint16_t> TransferOneFrame8_8RadarADC(const QString& filename);
std::vector<uint8_t> extractRaw12Frames8x8(const std::vector<uint8_t>& frame_bytes, size_t offset, size_t step);
std::vector<uint8_t> extractRaw12FromFrame8x8(const std::vector<uint8_t>& frame_bytes);



std::pair<size_t, size_t> TimeToFrameRange(double start, double end, size_t total_frames);

size_t readallmdftobin_by_frame(const std::string& filename,
                                const std::string& output_dir,
                                size_t req_start_frame,
                                size_t req_end_frame,
                                const std::string& video_channel_name = "VideoRawdata_VC0");

std::vector<FrameMetadata> readallmdftobin_to_files(const std::string& filename,
                                                    const std::string& output_dir,
                                                    double start_sec,
                                                    double end_sec,
                                                    const std::string& video_channel_name = "VideoRawdata_VC0") ;
#endif // MDF2BIN_H
