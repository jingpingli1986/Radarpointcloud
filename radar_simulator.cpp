#include "radar_simulator.h"
#include <QtMath>
#include <cstdlib>



RadarSimulator::RadarSimulator()
{
    std::srand(static_cast<unsigned>(time(nullptr)));
}

boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> RadarSimulator::generateScanFrame()
{
    auto cloud = boost::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
    float angleStart = -60.0f;
    float angleEnd = 60.0f;

    for (int i = 0; i < 1000; ++i) {
        float angle = angleStart + static_cast<float>(std::rand()) / RAND_MAX * (angleEnd - angleStart);
        float distance = 5.0f + static_cast<float>(std::rand()) / RAND_MAX * 15.0f;
        float rad = qDegreesToRadians(angle);

        pcl::PointXYZRGB pt;
        pt.x = distance * std::cos(rad);
        pt.y = distance * std::sin(rad);
        pt.z = distance * std::sin(rad);
        //pt.z = 0.0f;
        uint8_t intensity = static_cast<uint8_t>(255.0f * (1.0f - distance / 20.0f));
        pt.r = intensity;
        pt.g = 255 - intensity;
        pt.b = 128;

        cloud->points.push_back(pt);

    }

    cloud->width = static_cast<uint32_t>(cloud->points.size());
    cloud->height = 1;
    cloud->is_dense = false;

    return cloud;
}


boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> RadarSimulator::generate16LineScanFrame()
{
    auto cloud = boost::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();

    const int num_lines = 16;             // 16线雷达
    const int points_per_line = 360;      // 水平一圈360点，1度一测点
    const float vertical_fov_min = -15;   // 垂直下边界，单位度（可调）
    const float vertical_fov_max = 15;    // 垂直上边界，单位度（可调）
    const float distance_min = 1.0f;      // 最小测距，米
    const float distance_max = 50.0f;     // 最大测距，米

    for (int line = 0; line < num_lines; ++line)
    {
        // 计算每线对应的垂直角度
        float vertical_angle = vertical_fov_min + line * (vertical_fov_max - vertical_fov_min) / (num_lines - 1);
        float vertical_rad = qDegreesToRadians(vertical_angle);

        for (int i = 0; i < points_per_line; ++i)
        {
            float horizontal_angle = static_cast<float>(i); // 0 ~ 359度
            float horizontal_rad = qDegreesToRadians(horizontal_angle);

            // 模拟距离，带点随机噪声
            float distance = distance_min + static_cast<float>(std::rand()) / RAND_MAX * (distance_max - distance_min);

            // 球坐标转笛卡尔坐标
            pcl::PointXYZRGB pt;
            pt.x = distance * std::cos(vertical_rad) * std::cos(horizontal_rad);
            pt.y = distance * std::cos(vertical_rad) * std::sin(horizontal_rad);
            pt.z = distance * std::sin(vertical_rad);

            // 颜色根据距离渐变（简单处理）
            uint8_t intensity = static_cast<uint8_t>(255.0f * (1.0f - (distance - distance_min) / (distance_max - distance_min)));
            pt.r = intensity;
            pt.g = 255 - intensity;
            pt.b = 128;

            cloud->points.push_back(pt);
        }
    }

    cloud->width = static_cast<uint32_t>(cloud->points.size());
    cloud->height = 1;
    cloud->is_dense = false;

    return cloud;
}


