#include "pointcloud_loader.h"
#include "customradarPoint.h"
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellArray.h>

vtkSmartPointer<vtkPolyData> PointCloudLoader::convertToVTKPolyData(boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGB>> cloud)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    for (const auto& pt : cloud->points) {
        points->InsertNextPoint(pt.x, pt.y, pt.z);

        unsigned char color[3] = {pt.r, pt.g, pt.b};
        colors->InsertNextTypedTuple(color);
    }

    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);

    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
        vertices->InsertNextCell(1, &i);
    }
    polyData->SetVerts(vertices);

    polyData->GetPointData()->SetScalars(colors);

    return polyData;
}

// vtkSmartPointer<vtkPolyData> PointCloudLoader::convertmydataToVTKPolyData(const std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>& cloud)
// {
//     vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
//     points->SetDataTypeToFloat();
//     vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
//     colors->SetNumberOfComponents(3);
//     colors->SetName("RGB");

//     // 注册字段的 vtk 数组
//     vtkSmartPointer<vtkFloatArray> rangeArray       = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> dopplerArray     = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> powerdBArray     = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> snrArray         = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> qAziArray        = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> qEleArray        = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> azimuthAngArray  = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> eleAngArray      = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> radVelAbsArray      = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> detValidArray       = vtkSmartPointer<vtkFloatArray>::New();
//     vtkSmartPointer<vtkFloatArray> rcsdBArray       = vtkSmartPointer<vtkFloatArray>::New();

//     vtkSmartPointer<vtkIntArray> originalIdArray = vtkSmartPointer<vtkIntArray>::New();

//     vtkSmartPointer<vtkIntArray> frameIdArray = vtkSmartPointer<vtkIntArray>::New();
//     vtkSmartPointer<vtkFloatArray> heightArray = vtkSmartPointer<vtkFloatArray>::New();

//     // 设置名字（必须和注册的一致）
//     rangeArray->SetName("Range");
//     dopplerArray->SetName("DopplerSpeed");
//     powerdBArray->SetName("PowerdB");
//     snrArray->SetName("SNRdB");
//     qAziArray->SetName("Q_azi");
//     qEleArray->SetName("Q_ele");
//     azimuthAngArray->SetName("AzimuthAng");
//     eleAngArray->SetName("EleAng");
//     radVelAbsArray->SetName("radVelAbs");
//     detValidArray->SetName("detValid");
//     rcsdBArray->SetName("RcsdB");
//     originalIdArray->SetName("OriginalID");
//     originalIdArray->SetNumberOfComponents(1);

//     frameIdArray->SetName("FrameID");
//     frameIdArray->SetNumberOfComponents(1);

//     heightArray->SetName("height");

//     // double minAbs = std::numeric_limits<double>::max();
//     // double maxAbs = std::numeric_limits<double>::lowest();
//     // for (const auto& pt : cloud->points) {

//     //     if(pt.detValid>0){
//     //         double v = std::abs(pt.radVelAbs);
//     //         minAbs = std::min(minAbs, v);
//     //         maxAbs = std::max(maxAbs, v);
//     //     }
//     // }




//     for (const auto& pt : cloud->points) {
//         points->InsertNextPoint(pt.x, pt.y, pt.z);

//         // std::uint8_t r,g,b;
//         // if (std::abs(pt.radVelAbs) <= 1.0) {
//         //     r=0;
//         //     g=200;
//         //     b=0;

//         // }else{

//         //     double t = (std::abs(pt.radVelAbs) - minAbs) / (maxAbs - minAbs + 1e-9);
//         //     t = std::clamp(t, 0.0, 1.0);
//         //     t = t * t;

//         //     r = static_cast<unsigned char>(255 * t);  // 红色增加到黄色
//         //     g = static_cast<unsigned char>(165 * t);  // 绿色增加到黄色
//         //     b = static_cast<unsigned char>(255 * (1.0 - t)); // 蓝色减少

//         // }

//         unsigned char rgb[3] = {pt.r, pt.g, pt.b};
//         colors->InsertNextTypedTuple(rgb);


//         rangeArray->InsertNextValue(pt.range);
//         dopplerArray->InsertNextValue(pt.dopplerSpeed);
//         powerdBArray->InsertNextValue(pt.powerdB);
//         snrArray->InsertNextValue(pt.SNRdB);
//         qAziArray->InsertNextValue(pt.Q_azi);
//         qEleArray->InsertNextValue(pt.Q_ele);
//         azimuthAngArray->InsertNextValue(pt.azimuthAng);
//         eleAngArray->InsertNextValue(pt.eleAng);
//         radVelAbsArray->InsertNextValue(pt.radVelAbs);
//         detValidArray->InsertNextValue(pt.detValid);
//         rcsdBArray->InsertNextValue(pt.rcsdB);
//         originalIdArray->InsertNextValue(pt.originalIndex);
//         frameIdArray->InsertNextValue(pt.frameIndex);
//         heightArray->InsertNextValue(pt.z);
//     }

//     vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
//     polyData->SetPoints(points);

//     vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
//     for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
//         vertices->InsertNextCell(1, &i);
//     }
//     polyData->SetVerts(vertices);

//     polyData->GetPointData()->SetScalars(colors);
//     polyData->GetPointData()->AddArray(rangeArray);
//     polyData->GetPointData()->AddArray(dopplerArray);
//     polyData->GetPointData()->AddArray(powerdBArray);
//     polyData->GetPointData()->AddArray(snrArray);
//     polyData->GetPointData()->AddArray(qAziArray);
//     polyData->GetPointData()->AddArray(qEleArray);
//     polyData->GetPointData()->AddArray(azimuthAngArray);
//     polyData->GetPointData()->AddArray(eleAngArray);
//     polyData->GetPointData()->AddArray(radVelAbsArray);
//     polyData->GetPointData()->AddArray(detValidArray);
//     polyData->GetPointData()->AddArray(rcsdBArray);

//     polyData->GetPointData()->AddArray(originalIdArray);
//     polyData->GetPointData()->AddArray(frameIdArray);
//     polyData->GetPointData()->AddArray(heightArray);
//     return polyData;
// }

vtkSmartPointer<vtkPolyData> PointCloudLoader::convertmydataToVTKPolyData(const std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>>& cloud)
{
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    if (!cloud || cloud->empty()) return polyData;

    const size_t numPoints = cloud->size();

    // --- 1. 预分配所有数组内存 ---
    // 坐标
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToFloat();
    points->SetNumberOfPoints(numPoints);
    float* pPoints = static_cast<float*>(points->GetData()->GetVoidPointer(0));

    // 颜色 (RGB)
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetName("RGB");
    colors->SetNumberOfTuples(numPoints);
    unsigned char* pColors = static_cast<unsigned char*>(colors->GetVoidPointer(0));

    // 各种浮点字段属性
    auto createFloatArray = [&](const char* name) {
        vtkSmartPointer<vtkFloatArray> arr = vtkSmartPointer<vtkFloatArray>::New();
        arr->SetName(name);
        arr->SetNumberOfTuples(numPoints);
        return arr;
    };

    auto rangeArray       = createFloatArray("Range");
    auto dopplerArray     = createFloatArray("DopplerSpeed");
    auto powerdBArray     = createFloatArray("PowerdB");
    auto snrArray         = createFloatArray("SNRdB");
    auto qAziArray        = createFloatArray("Q_azi");
    auto qEleArray        = createFloatArray("Q_ele");
    auto azimuthAngArray  = createFloatArray("AzimuthAng");
    auto eleAngArray      = createFloatArray("EleAng");
    auto radVelAbsArray   = createFloatArray("radVelAbs");
    auto detValidArray    = createFloatArray("detValid");
    auto rcsdBArray       = createFloatArray("RcsdB");
    auto heightArray      = createFloatArray("height");

    // 整型字段属性
    vtkSmartPointer<vtkIntArray> originalIdArray = vtkSmartPointer<vtkIntArray>::New();
    originalIdArray->SetName("OriginalID");
    originalIdArray->SetNumberOfTuples(numPoints);

    vtkSmartPointer<vtkIntArray> frameIdArray = vtkSmartPointer<vtkIntArray>::New();
    frameIdArray->SetName("FrameID");
    frameIdArray->SetNumberOfTuples(numPoints);

    //增加两个字段属性rangebin 和 velbin
    vtkSmartPointer<vtkIntArray> rangebinArray = vtkSmartPointer<vtkIntArray>::New();
    rangebinArray->SetName("Rangebin");
    rangebinArray->SetNumberOfTuples(numPoints);

    vtkSmartPointer<vtkIntArray> velbinArray = vtkSmartPointer<vtkIntArray>::New();
    velbinArray->SetName("Velbin");
    velbinArray->SetNumberOfTuples(numPoints);



    // 获取所有属性数组的原始指针，用于极速写入
    float* ptrRange   = static_cast<float*>(rangeArray->GetVoidPointer(0));
    float* ptrDoppler = static_cast<float*>(dopplerArray->GetVoidPointer(0));
    float* ptrPower   = static_cast<float*>(powerdBArray->GetVoidPointer(0));
    float* ptrSNR     = static_cast<float*>(snrArray->GetVoidPointer(0));
    float* ptrQAzi    = static_cast<float*>(qAziArray->GetVoidPointer(0));
    float* ptrQEle    = static_cast<float*>(qEleArray->GetVoidPointer(0));
    float* ptrAziAng  = static_cast<float*>(azimuthAngArray->GetVoidPointer(0));
    float* ptrEleAng  = static_cast<float*>(eleAngArray->GetVoidPointer(0));
    float* ptrRadVel  = static_cast<float*>(radVelAbsArray->GetVoidPointer(0));
    float* ptrDetVal  = static_cast<float*>(detValidArray->GetVoidPointer(0));
    float* ptrRCS     = static_cast<float*>(rcsdBArray->GetVoidPointer(0));
    float* ptrHeight  = static_cast<float*>(heightArray->GetVoidPointer(0));
    int* ptrOrigId  = static_cast<int*>(originalIdArray->GetVoidPointer(0));
    int* ptrFrameId = static_cast<int*>(frameIdArray->GetVoidPointer(0));

    //额外增加两个属性
    int* ptrrangebin  = static_cast<int*>(rangebinArray->GetVoidPointer(0));
    int* ptrvelbin = static_cast<int*>(velbinArray->GetVoidPointer(0));

    // --- 2. 核心循环：指针直写 ---
    for (size_t i = 0; i < numPoints; ++i) {
        const auto& pt = cloud->points[i];

        // 坐标
        size_t i3 = i * 3;
        pPoints[i3]     = pt.x;
        pPoints[i3 + 1] = pt.y;
        pPoints[i3 + 2] = pt.z;

        // 颜色
        pColors[i3]     = pt.r;
        pColors[i3 + 1] = pt.g;
        pColors[i3 + 2] = pt.b;

        // 属性直写
        ptrRange[i]   = pt.range;
        ptrDoppler[i] = pt.dopplerSpeed;
        ptrPower[i]   = pt.powerdB;
        ptrSNR[i]     = pt.SNRdB;
        ptrQAzi[i]    = pt.Q_azi;
        ptrQEle[i]    = pt.Q_ele;
        ptrAziAng[i]  = pt.azimuthAng;
        ptrEleAng[i]  = pt.eleAng;
        ptrRadVel[i]  = pt.radVelAbs;
        ptrDetVal[i]  = pt.detValid;
        ptrRCS[i]     = pt.rcsdB;
        ptrHeight[i]  = pt.z;
        ptrOrigId[i]  = pt.originalIndex;
        ptrFrameId[i] = pt.frameIndex;

        //增加两个属性写入
        ptrrangebin[i] = pt.rangebin;
        ptrvelbin[i] = pt.velbin;
    }

    // --- 3. 拓扑结构构建 (Vertex Cells) ---
    // 性能点：使用预分配的 CellArray 避免重复插入
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkIdTypeArray> conn = vtkSmartPointer<vtkIdTypeArray>::New();
    conn->SetNumberOfTuples(numPoints * 2); // 每个顶点占 2 个位置 (size + id)
    vtkIdType* pConn = static_cast<vtkIdType*>(conn->GetVoidPointer(0));
    for (vtkIdType i = 0; i < (vtkIdType)numPoints; ++i) {
        pConn[i * 2] = 1;     // Cell 大小为 1
        pConn[i * 2 + 1] = i; // 顶点 ID
    }
    vertices->SetCells(numPoints, conn);

    // --- 4. 组装 PolyData ---
    polyData->SetPoints(points);
    polyData->SetVerts(vertices);

    auto pd = polyData->GetPointData();
    pd->SetScalars(colors);
    pd->AddArray(rangeArray);
    pd->AddArray(dopplerArray);
    pd->AddArray(powerdBArray);
    pd->AddArray(snrArray);
    pd->AddArray(qAziArray);
    pd->AddArray(qEleArray);
    pd->AddArray(azimuthAngArray);
    pd->AddArray(eleAngArray);
    pd->AddArray(radVelAbsArray);
    pd->AddArray(detValidArray);
    pd->AddArray(rcsdBArray);
    pd->AddArray(originalIdArray);
    pd->AddArray(frameIdArray);
    pd->AddArray(heightArray);

    //add 两个属性
    pd->AddArray(rangebinArray);
    pd->AddArray(velbinArray);

    return polyData;
}
