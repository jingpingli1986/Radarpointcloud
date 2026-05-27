#include "pointcloudcolorizer.h"
#include "QDebug"
pointcloudcolorizer::pointcloudcolorizer() {}


// ----------------------------------------------------
// 辅助函数：根据归一化值 t 计算 RGB (保持不变)
// ----------------------------------------------------
void pointcloudcolorizer::calculateColor(double t, unsigned char& r, unsigned char& g, unsigned char& b) {
    // 示例色带：从蓝色 (低值) 渐变到红色 (高值)
    t = std::clamp(t, 0.0, 1.0);
    t = t * t; // 示例：使低值更集中

    // BGR 渐变: Blue(低) -> Green -> Red(高)
    if (t < 0.5) {
        // 蓝色到绿色 (t=0 -> t=0.5)
        r = 0;
        g = static_cast<unsigned char>(t * 2 * 255);
        b = static_cast<unsigned char>(255 - t * 2 * 255);
    } else {
        // 绿色到红色 (t=0.5 -> t=1.0)
        r = static_cast<unsigned char>((t - 0.5) * 2 * 255);
        g = static_cast<unsigned char>(255 - (t - 0.5) * 2 * 255);
        b = 0;
    }
}

// ----------------------------------------------------
// 核心着色逻辑 (移除 detValid 判断)
// ----------------------------------------------------
void pointcloudcolorizer::colorizePointCloud(CloudTPtr cloud, ColorMode mode) {
    if (!cloud || cloud->points.empty()) return;

    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    // 第一次遍历：确定属性的最小/最大值 (针对所有点)
    for (const auto& pt : cloud->points) {
        double v = 0.0;

        switch (mode) {
        case ColorMode::RadialVelocity:
            v = std::abs(pt.radVelAbs);
            break;
        case ColorMode::RCS:
            v = pt.rcsdB;
            break;
        case ColorMode::Height:
            v = pt.z;
            break;
        case ColorMode::SNR:
            v = pt.SNRdB;
            break;
        case ColorMode::Fixed:
            break;
        }

        if (mode != ColorMode::Fixed) {
            minVal = std::min(minVal, v);
            maxVal = std::max(maxVal, v);
        }
    }

    // 检查并处理范围为零的情况
    if (mode != ColorMode::Fixed && std::abs(maxVal - minVal) < 1e-6) {
        maxVal = minVal + 1.0;
    }

    // 确保 maxVal > 0 (特别是速度，避免除以负数或零)
    double range = maxVal - minVal;

    // *** 关键调试日志 ***
    // qDebug() << "ColorMode:" << static_cast<int>(mode);
    // qDebug() << "Min/Max Val:" << minVal << maxVal;
    // qDebug() << "Range:" << range;


    // 第二次遍历：着色 (针对所有点)
    for (auto& pt : cloud->points) {

        if (mode == ColorMode::Fixed) {
            pt.r = 0; pt.g = 0; pt.b = 0; // 💥 固定为白色 💥 (修正：原代码为黑色)
            continue;
        }

        double val = 0.0;
        unsigned char r=0, g=0, b=0;

        switch (mode) {
        case ColorMode::RadialVelocity: {
            val = std::floor(std::abs(pt.radVelAbs));

            // 策略：0值点设为背景色（例如白色），低速绿色，高速蓝-红渐变
            if (val == 0.0) {
                //r = 255; g = 255; b = 255; // 白色 (忽略大量静止点)
                r = 0; g = 200; b = 0;
            } else if (val <= 1.0) {
                r = 0; g = 200; b = 0; // 绿色 (极低速)
            } else {
                // 对 (1.0, maxVal] 范围内的值进行映射
                double t_eff = (val - 1.0) / (maxVal - 1.0 + 1e-9);
                t_eff = std::clamp(t_eff, 0.0, 1.0);
                t_eff = std::sqrt(t_eff); // 使用平方根强调高速点

                // 蓝(低速) -> 红(高速)
                r = static_cast<unsigned char>(255 * t_eff);
                g = 0;
                b = static_cast<unsigned char>(255 * (1.0 - t_eff));
            }
            break;
        }

        case ColorMode::RCS: {
            val = pt.rcsdB;
            // 策略：灰度着色，高RCS更亮，低RCS更暗（或反色）
            double t = (val - minVal) / (range + 1e-9);
            t = std::clamp(t, 0.0, 1.0);

            // 从黑色 (低RCS) 到白色 (高RCS)
            unsigned char gray = static_cast<unsigned char>(255 * t);
            r = gray; g = gray; b = gray;
            break;
        }

        case ColorMode::Height: {
            val = pt.z;
            // 策略：地形图色带，蓝(低) -> 绿(中) -> 红(高)
            double t = (val - minVal) / (range + 1e-9);
            t = std::clamp(t, 0.0, 1.0);

            // 从蓝到红的连续渐变
            if (t < 0.5) {
                // 蓝(t=0) 到 绿(t=0.5)
                r = 0;
                g = static_cast<unsigned char>(t * 2 * 255);
                b = static_cast<unsigned char>(255 - t * 2 * 255);
            } else {
                // 绿(t=0.5) 到 红(t=1.0)
                r = static_cast<unsigned char>((t - 0.5) * 2 * 255);
                g = static_cast<unsigned char>(255 - (t - 0.5) * 2 * 255);
                b = 0;
            }
            break;
        }


        case ColorMode::SNR: {
            val = pt.SNRdB;
            // 策略：地形图色带，蓝(低) -> 绿(中) -> 红(高)
            double t = (val - minVal) / (range + 1e-9);
            t = std::clamp(t, 0.0, 1.0);

            // 从蓝到红的连续渐变
            if (t < 0.5) {
                // 蓝(t=0) 到 绿(t=0.5)
                r = 0;
                g = static_cast<unsigned char>(t * 2 * 255);
                b = static_cast<unsigned char>(255 - t * 2 * 255);
            } else {
                // 绿(t=0.5) 到 红(t=1.0)
                r = static_cast<unsigned char>((t - 0.5) * 2 * 255);
                g = static_cast<unsigned char>(255 - (t - 0.5) * 2 * 255);
                b = 0;
            }
            break;
        }

        case ColorMode::Fixed :{
            r = 0; g = 0; b = 0; // 默认黑色
            break;
        }
        default:
            r = 0; g = 0; b = 0; // 默认黑色
            break;
        }

        // if (&pt == &cloud->points[0]) {
        //     qDebug() << "First Point Color (R,G,B):" << (int)r << (int)g << (int)b;
        // }

        pt.r = r;
        pt.g = g;
        pt.b = b;

        // -------------------------------------------------------------------
        // *** 关键修正：确保打包的 float rgb 字段也得到更新 ***
        // -------------------------------------------------------------------

        // // PCL 标准操作：将 R, G, B 打包成 uint32_t 颜色整数 (忽略 Alpha/Padding)
        // uint32_t rgb_packed = (static_cast<uint32_t>(pt.r) << 16 |
        //                        static_cast<uint32_t>(pt.g) << 8 |
        //                        static_cast<uint32_t>(pt.b));

        // // 使用 memcpy 将 uint32_t 的内存内容安全地复制到 float rgb 字段中
        // // 确保了 float 字段得到了正确的内存表示。
        // memcpy(&(pt.rgb), &rgb_packed, sizeof(uint32_t));

    }
}
