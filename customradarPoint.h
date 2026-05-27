#ifndef CUSTOMRADARPOINT_H
#define CUSTOMRADARPOINT_H
#include "pcl/visualization/point_picking_event.h"
#include <pcl/point_types.h>
#include <cstdint>
#include <pcl/point_cloud.h>

#include <boost/make_shared.hpp>
#include <pcl/visualization/point_cloud_handlers.h>
#include <pcl/visualization/impl/point_cloud_geometry_handlers.hpp>
#include <vector>


struct  PointXYZRGBWithProperties : public pcl::PointXYZRGB {
    float range;
    float dopplerSpeed;
    float powerdB;
    float SNRdB;
    float Q_azi;
    float Q_ele;
    float azimuthAng;
    float eleAng;
    float radVelAbs;
    float detValid;
    float rcsdB;
    int originalIndex = -1;
    int frameIndex = -1;
    int rangebin = -1;
    int velbin = -1;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

POINT_CLOUD_REGISTER_POINT_STRUCT(PointXYZRGBWithProperties,
                                  (float, x, x)
                                  (float, y, y)
                                  (float, z, z)
                                  (uint8_t, r, r)
                                  (uint8_t, g, g)
                                  (uint8_t, b, b)
                                  (float, range, range)
                                  (float, dopplerSpeed, dopplerSpeed)
                                  (float, powerdB, powerdB)
                                  (float, SNRdB, SNRdB)
                                  (float, Q_azi, Q_azi)
                                  (float, Q_ele, Q_ele)
                                  (float, azimuthAng, azimuthAng)
                                  (float, eleAng, eleAng)
                                  (float, radVelAbs, radVelAbs)
                                  (float, detValid, detValid)
                                  (float, rcsdB, rcsdB)
                                  (int, originalIndex, originalIndex)
                                  (int, frameIndex, frameIndex)
                                  )





class Customcradar
{
public:
    Customcradar();
    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> SateliteRadarData();
    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> SateliteRadarData(std::string filename);
    static void pp_callback(const pcl::visualization::PointPickingEvent& event, void* args);
};



#endif // CUSTOMRADARPOINT_H
