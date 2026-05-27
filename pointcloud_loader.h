#ifndef POINTCLOUD_LOADER_H
#define POINTCLOUD_LOADER_H
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <boost/make_shared.hpp>
#include "customradarPoint.h"

class PointCloudLoader
{
public:
    static vtkSmartPointer<vtkPolyData> convertToVTKPolyData(boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> cloud);

    static vtkSmartPointer<vtkPolyData> convertmydataToVTKPolyData(const std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>& cloud);
};

#endif // POINTCLOUD_LOADER_H
