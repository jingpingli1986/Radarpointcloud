#ifndef POINTCLOUDCOLORIZER_H
#define POINTCLOUDCOLORIZER_H
#include "customradarPoint.h"

using CloudT = pcl::PointCloud<PointXYZRGBWithProperties>;
using CloudTPtr = std::shared_ptr<CloudT>;

enum class ColorMode {
    RadialVelocity, // 基于 pt.radVelAbs
    RCS,            // 基于 pt.rcsdB
    Height,         // 基于 pt.z (高度)
    SNR,            // 基于pt.snr
    Fixed           // 固定颜色 (例如白色)
};


class pointcloudcolorizer
{
public:
    pointcloudcolorizer();
    void colorizePointCloud(CloudTPtr cloud, ColorMode mode);


private:
    void calculateColor(double t, unsigned char& r, unsigned char& g, unsigned char& b);
};

#endif // POINTCLOUDCOLORIZER_H
