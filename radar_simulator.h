#ifndef RADAR_SIMULATOR_H
#define RADAR_SIMULATOR_H

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <boost/make_shared.hpp>
#include <pcl/register_point_struct.h>

class RadarSimulator
{
public:
    RadarSimulator();
    boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> generateScanFrame();
    boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> generate16LineScanFrame();

};

#endif // RADAR_SIMULATOR_H
