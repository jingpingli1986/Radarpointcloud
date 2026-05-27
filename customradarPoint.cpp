#include "customradarPoint.h"
#include <QtMath>
#include <cstdlib>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/point_cloud.h>
#include <iostream>
#include <cstring>
#include <ctime>

#include <QDebug>
#include <QString>
#include <memory>  // 使用 std::shared_ptr，替代 boost::shared_ptr

#include <pcl/visualization/point_cloud_handlers.h>
#include <pcl/impl/instantiate.hpp>
#include <pcl/point_traits.h>

// 模板实例化
template class pcl::visualization::PointCloudGeometryHandlerXYZ<PointXYZRGBWithProperties>;
template class pcl::visualization::PointCloudColorHandlerRGBField<PointXYZRGBWithProperties>;

using PointT = PointXYZRGBWithProperties;
using PointCloudT = pcl::PointCloud<PointT>;

Customcradar::Customcradar()
{
    std::srand(static_cast<unsigned>(time(nullptr)));
}

void Customcradar::pp_callback(const pcl::visualization::PointPickingEvent& event, void* args)
{
    auto cloud = *static_cast<std::shared_ptr<PointCloudT>*>(args);
    if (event.getPointIndex() == -1)
        return;

    int idx = event.getPointIndex();
    if (idx >= cloud->points.size())
        return;

    const PointT& pt = cloud->points[idx];
    qDebug() << "Picked point index: " << idx;
    qDebug() << "Coordinates: (" << pt.x << ", " << pt.y << ", " << pt.z << ")";

}

std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> Customcradar::SateliteRadarData()
{
    std::string filename = "output.csv";
    std::ifstream file(filename);
    if (!file.is_open()) {
        qDebug() << "无法打开CSV文件: " << QString::fromStdString(filename);
        return std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>();
    }
    auto cloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string value;
        PointXYZRGBWithProperties pt;

        try {
            std::getline(iss, value, ','); pt.range = std::stof(value);
            std::getline(iss, value, ','); pt.dopplerSpeed = std::stof(value);
            std::getline(iss, value, ','); pt.powerdB = std::stof(value);
            std::getline(iss, value, ','); pt.SNRdB = std::stof(value);
            std::getline(iss, value, ','); pt.Q_azi = std::stof(value);
            std::getline(iss, value, ','); pt.Q_ele = std::stof(value);
            std::getline(iss, value, ','); pt.azimuthAng = std::stof(value);
            std::getline(iss, value, ','); pt.eleAng = std::stof(value);
            std::getline(iss, value, ','); pt.radVelAbs = std::stof(value);
            std::getline(iss, value, ','); pt.x = std::stof(value);
            std::getline(iss, value, ','); pt.y = std::stof(value);
            std::getline(iss, value, ','); pt.z = std::stof(value);

            // 跳过这4列
            for (int i = 0; i < 3; ++i) std::getline(iss, value, ',');

            std::getline(iss, value, ','); pt.detValid = std::stof(value);
            std::getline(iss, value, ','); pt.rcsdB = std::stof(value);

            // 设置主坐标


            // 设置颜色（可根据属性变化赋值）
            pt.r = 255;
            pt.g = 255;
            pt.b = 255;

            cloud->points.push_back(pt);
        }
        catch (...) {
            qDebug() << "CSV 行解析失败，跳过该行";
            continue;
        }
    }

    cloud->width = static_cast<uint32_t>(cloud->points.size());
    cloud->height = 1;
    cloud->is_dense = true;



    return cloud;
}


std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> Customcradar::SateliteRadarData(std::string filename)
{

    int currentOriginalIndex = 0;

    std::ifstream file(filename);
    if (!file.is_open()) {
        qDebug() << "无法打开CSV文件: " << QString::fromStdString(filename);
        return std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>();
    }
    auto cloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string value;
        PointXYZRGBWithProperties pt;

        try {
            std::getline(iss, value, ','); pt.range = std::stof(value);
            std::getline(iss, value, ','); pt.dopplerSpeed = std::stof(value);
            std::getline(iss, value, ','); pt.powerdB = std::stof(value);
            std::getline(iss, value, ','); pt.SNRdB = std::stof(value);
            std::getline(iss, value, ','); pt.Q_azi = std::stof(value);
            std::getline(iss, value, ','); pt.Q_ele = std::stof(value);
            std::getline(iss, value, ','); pt.azimuthAng = std::stof(value);
            std::getline(iss, value, ','); pt.eleAng = std::stof(value);
            std::getline(iss, value, ','); pt.radVelAbs = std::stof(value);
            std::getline(iss, value, ','); pt.x = std::stof(value);
            std::getline(iss, value, ','); pt.y = std::stof(value);
            std::getline(iss, value, ','); pt.z = std::stof(value);

            // 跳过这4列
            for (int i = 0; i < 3; ++i) std::getline(iss, value, ',');

            std::getline(iss, value, ','); pt.detValid = std::stof(value);
            std::getline(iss, value, ','); pt.rcsdB = std::stof(value);

            pt.originalIndex = currentOriginalIndex;
            // 设置主坐标


            // 设置颜色（可根据属性变化赋值）
            pt.r = 255;
            pt.g = 255;
            pt.b = 255;

            cloud->points.push_back(pt);
            currentOriginalIndex++;
        }
        catch (...) {
            qDebug() << "CSV 行解析失败，跳过该行";
            continue;
        }
    }

    cloud->width = static_cast<uint32_t>(cloud->points.size());
    cloud->height = 1;
    cloud->is_dense = true;



    return cloud;
}
