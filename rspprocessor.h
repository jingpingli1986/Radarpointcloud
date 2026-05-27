#ifndef RSPPROCESSOR_H
#define RSPPROCESSOR_H

#include <QLibrary>
#include <QDebug>
#include "dspProssing.h" // 包含头文件，获取类型定义

// 1. 定义函数指针类型 (必须在全局可见)
typedef void (*RspProssingFunc)(
    float* ptTimedata,
    VehiclesInfo_t* ptVehInfo,
    RadarInfo_t *ptRadarInfo,
    PointCloud2_t* ptPointCloud,
    float* ptRdmap
    );

class RspProcessor {
public:
    // 静态方法，用于获取函数指针
    static RspProssingFunc getRspProssingFunction();

private:
    // 构造函数私有化，确保它不能被外部实例化
    RspProcessor();

    // 拷贝和赋值删除
    RspProcessor(const RspProcessor&) = delete;
    RspProcessor& operator=(const RspProcessor&) = delete;

    // 静态成员，用于存储 QLibrary 实例和解析后的函数指针
    static QLibrary library;
    static RspProssingFunc funcPtr;
    static bool initialized;

    // 内部初始化函数
    static void initialize();
};

#endif // RSPPROCESSOR_H
