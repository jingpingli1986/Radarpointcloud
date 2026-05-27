#include "rspprocess.h"
#include "RadarBinAnalysis.h"


// ------------------------------------------------------------------
// 辅助函数：将归一化值 [0, 1] 映射到 Jet Colormap 颜色
// ------------------------------------------------------------------
QColor jetColormap(double normVal)
{
    // 确保值在 [0, 1] 范围内
    normVal = qBound(0.0, normVal, 1.0);

    // Jet Colormap 算法（将 [0, 1] 映射到 [R, G, B]）
    double x = normVal * 4.0;
    double r = qBound(0.0, x - 1.5, 1.0);
    double g = qBound(0.0, qMin(x - 0.5, 2.5 - x), 1.0);
    double b = qBound(0.0, qMin(x + 0.5, 4.5 - x), 1.0);

    // 转换为 0-255
    return QColor((int)(r * 255), (int)(g * 255), (int)(b * 255));
}


// QImage dspdll_Test(){

//     QLibrary lib("C:/Users/JPN1WX/Desktop/QT/PointCloudUI/rspProcessing.dll");
//     if (!lib.load()) {
//         qDebug() << "Failed to load DLL:" << lib.errorString();
//         return QImage();
//     }

//     RspProssingFunc rspProssing = (RspProssingFunc)lib.resolve("rspProssing");
//     if (!rspProssing) {
//         qDebug() << "Failed to resolve function:" << lib.errorString();
//         return QImage();
//     }
//     float* timedata = nullptr;
//     VehiclesInfo_t vehInfo = {};
//     RadarInfo_t radarInfo = {};
//     PointCloud2_t pointCloud = {};
//     // uint32_t rdmap[RDMAP_SIZE] = {0};
//     float* rdmap = new float[RDMAP_SIZE];
//     rspProssing(timedata, &vehInfo, &radarInfo, &pointCloud, rdmap);
//     // ... (qDebugs) ...

//     // 这一行可以删除或注释掉，因为我们将使用 QCustomPlot 生成图像
//     QImage rdMapImage = createRangeDopplerImage(rdmap);




//     return rdMapImage;
// }


// QImage createRangeDopplerImage(float* rdmap)
// {
//     // 1. 查找最大/最小值
//     float maxVal = 0;
//     float minVal = (RDMAP_SIZE > 0) ? rdmap[0] : 0;
//     for (int i = 0; i < RDMAP_SIZE; ++i) {
//         if (rdmap[i] > maxVal) maxVal = rdmap[i];
//         if (rdmap[i] < minVal) minVal = rdmap[i];
//     }
//     float range = maxVal - minVal;

//     // 2. 创建 QImage
//     // 注意：我们将 Range (512) 作为高度，Doppler (1024) 作为宽度
//     QImage rdImage(FFT_2D_SIZE, RNG_NUM, QImage::Format_RGB32);

//     for (int i = 0; i < RNG_NUM; ++i) { // 距离 (行/Y)
//         for (int j = 0; j < FFT_2D_SIZE; ++j) { // 多普勒 (列/X)

//             int index = i * FFT_2D_SIZE + j;
//             float value = rdmap[index];

//             // 3. 归一化和颜色映射 (使用 Jet Colormap)
//             double normVal = 0.0;
//             if (range > 0) {
//                 // 归一化到 [0, 1]
//                 normVal = (double)(value - minVal) / range;
//             }

//             // 使用 Jet Colormap
//             QColor color = jetColormap(normVal);

//             // 4. 设置像素颜色
//             // 图像的 Y 轴方向通常是从上到下，距离轴通常是反转的 (近距离在底部)
//             // 如果希望近距离 (i=0) 在图像底部，可以反转 Y 索引：
//             int y_index = RNG_NUM - 1 - i;

//             rdImage.setPixel(j, y_index, color.rgb());
//         }
//     }

//     return rdImage;
// }


// // 假设这是您的处理函数，现在它接受一个 QCustomPlot 指针
// void setupRangeDopplerPlot(QCustomPlot *customPlot) {
//     // ... (DLL loading, rspProssing call, rdmap setup/call code remains the same) ...
//     QLibrary lib("C:/Users/JPN1WX/Desktop/QT/PointCloudUI/rspProcessing.dll");
//     if (!lib.load()) {
//         qDebug() << "Failed to load DLL:" << lib.errorString();
//         return ;
//     }

//     RspProssingFunc rspProssing = (RspProssingFunc)lib.resolve("rspProssing");
//     if (!rspProssing) {
//         qDebug() << "Failed to resolve function:" << lib.errorString();
//         return;
//     }


//     // 1. 确保 customPlot 非空
//     if (!customPlot) {
//         qDebug() << "Error: QCustomPlot pointer is null.";
//         return;
//     }

//     // 2. 清理旧图层和数据 (重要，防止重复积累)
//     customPlot->clearPlottables();
//     customPlot->clearGraphs();

//     // ... (DLL加载和数据获取逻辑，这里简化)
//     // 假设 rdmap 数据已经成功获取并填充
//     qDebug()<<"start";
//     float* timedata = readBinToFloatPtr();
//     VehiclesInfo_t vehInfo ;
//     vehInfo.speed = 0.0f;       // 例如，设置为 10.5 m/s
//     vehInfo.yawrate = 0.0f;      // 例如，设置为 0.1 rad/s
//     vehInfo.acceleration = 0.0f; // 例如，设置为 1.0 m/s^2

//     // 初始化枚举成员
//     vehInfo.gearInfo = D;        // 例如，设置为前进挡 (D)

//     RadarInfo_t radarInfo;
//     radarInfo.radarId = Front;
//     radarInfo.frameCount = 1; // 例如，设置为帧计数 100

//     // 初始化 double 数组 (timeStamp[100])
//     // 建议至少初始化第一个元素，防止 DLL 内部访问到未初始化数据
//     for (int i = 0; i < 1; ++i) {
//         radarInfo.timeStamp[i] = 0.0; // 全部初始化为 0.0
//     }

//     // 初始化嵌套结构体 MountenInfo_t
//     radarInfo.mountenInfo.mountenR = 0.0f;
//     radarInfo.mountenInfo.mountenYaw = 0.0f;
//     radarInfo.mountenInfo.mountenPitch=0.0f;
//     radarInfo.mountenInfo.mountenRoll=0.0f;

//     PointCloud2_t* pointCloud = new PointCloud2_t;
//     if (pointCloud == nullptr) {
//         qCritical() << "Error: Failed to allocate memory for PointCloud2_t.";
//         // 这里应该加入资源清理和错误处理
//         return;
//     }
//     // uint32_t rdmap[RDMAP_SIZE] = {0};
//     float* rdmap = new float[RDMAP_SIZE];
//     qDebug()<<"end";
//     rspProssing(timedata, &vehInfo, &radarInfo, pointCloud, rdmap);
//     qDebug()<<"end";


//     QCPColorMap *colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
//     // customPlot->setMinimumSize(FFT_2D_SIZE, RNG_NUM);

//     // 初始化手动范围变量
//     double calculatedMin = std::numeric_limits<double>::max();
//     double calculatedMax = std::numeric_limits<double>::min();

//     // 1. 设置尺寸并填充数据，同时进行 dB 转换
//     colorMap->data()->setSize(FFT_2D_SIZE, RNG_NUM);

//     for (int i = 0; i < RNG_NUM; ++i) {
//         for (int j = 0; j < FFT_2D_SIZE; ++j) {
//             int index = i * FFT_2D_SIZE + j;
//             double powerValue = (double)rdmap[index];

//             double dbValue;
//             if (powerValue > 0) {
//                 // 使用换底公式确保正确的 dB 转换
//                 dbValue = 10.0 * (std::log(powerValue) / std::log(10.0));
//             } else {
//                 dbValue = -999.0;
//             }

//             colorMap->data()->setCell(j, i, dbValue);

//             // ** 在这里手动更新数据的 Min/Max 范围 **
//             if (dbValue > calculatedMax) calculatedMax = dbValue;
//             if (dbValue < calculatedMin) calculatedMin = dbValue;
//         }
//     }

//     // 2. 设置颜色梯度
//     colorMap->setGradient(QCPColorGradient(QCPColorGradient::gpJet));

//     // 3. 使用手动计算的正确范围
//     QCPRange dbRangeManual(calculatedMin, calculatedMax);

//     // 4. 设置颜色映射的显示范围 (Z 轴 - dB 值)
//     QCPRange displayRange;
//     if (dbRangeManual.upper > dbRangeManual.lower) {
//         // 设置一个合理的动态范围 (通常 40-60 dB 足够显示目标和背景)
//         double dynamicRange = 50.0;
//         double minDisplayDb = dbRangeManual.upper - dynamicRange;

//         // 确保显示的最小值不会低于实际数据的最小值
//         if (minDisplayDb < dbRangeManual.lower) {
//             minDisplayDb = dbRangeManual.lower;
//         }

//         displayRange = QCPRange(minDisplayDb, dbRangeManual.upper);
//     } else {
//         // 容错处理，使用一个接近 40 dB 的默认范围
//         displayRange = QCPRange(0.0, 40.0);
//     }

//     colorMap->setDataRange(displayRange);

//     colorMap->data()->setKeyRange(QCPRange(0, FFT_2D_SIZE)); // X轴范围 0 到 1024
//     colorMap->data()->setValueRange(QCPRange(0, RNG_NUM));     // Y轴范围 0 到 512

//     // 5. 实例化和设置 QCPColorScale
//     QCPColorScale *colorScale = new QCPColorScale(customPlot);
//     customPlot->plotLayout()->addElement(0, 1, colorScale);

//     // 颜色条类型设置 (根据 QCP 2.1.1 定义)
//     colorScale->setType(QCPAxis::atRight);

//     colorMap->setColorScale(colorScale);
//     colorScale->setDataRange(displayRange); // 使用修正后的显示范围

//     // 6. 设置轴标签和范围 (X/Y 轴)
//     customPlot->xAxis->setLabel("Doppler/Velocity Bins (FFT_2D_SIZE=1024)");
//     customPlot->yAxis->setLabel("Range Bins (RNG_NUM=512)");
//     customPlot->xAxis->setRange(0, FFT_2D_SIZE);
//     customPlot->yAxis->setRange(0, RNG_NUM);
//     customPlot->axisRect()->setupFullAxesBox(true);

//     // 7. 绘图
//     // customPlot->rescaleAxes();
//     customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//     customPlot->replot();

//     delete[] rdmap;
//     delete[] timedata;
//     delete pointCloud;
//     // 注意：不再需要 delete customPlot，因为它是由 MainWindow 拥有的
// }

// void setupRangeDopplerPlot3D(Q3DSurface *graph){


//     // ... (DLL loading, rspProssing call, rdmap setup/call code remains the same) ...
//     QLibrary lib("C:/Users/JPN1WX/Desktop/QT/PointCloudUI/rspProcessing.dll");
//     if (!lib.load()) {
//         qDebug() << "Failed to load DLL:" << lib.errorString();
//         return ;
//     }

//     RspProssingFunc rspProssing = (RspProssingFunc)lib.resolve("rspProssing");
//     if (!rspProssing) {
//         qDebug() << "Failed to resolve function:" << lib.errorString();
//         return;
//     }


//     // ... (DLL加载和数据获取逻辑，这里简化)
//     // 假设 rdmap 数据已经成功获取并填充

//     float * timedata = nullptr;
//     VehiclesInfo_t vehInfo ;

//     RadarInfo_t radarInfo = {};
//     PointCloud2_t pointCloud = {};
//     // uint32_t rdmap[RDMAP_SIZE] = {0};
//     float* rdmap = new float[RDMAP_SIZE];
//     if (!rspProssing) {
//         qDebug() << "ERROR: rspProssing function not resolved!"; // 检查这里
//         return; // 中断执行
//     }

//     rspProssing(timedata, &vehInfo, &radarInfo, &pointCloud, rdmap);

//     if (rdmap == nullptr || rdmap[0] == 0) {
//         qDebug() << "WARNING: RD Map data is empty or null!"; // 检查这里
//         // 确保你的数据循环不会在空数据上运行
//     }

//     // --- 假设数据已成功加载到 rdmap ---

//     if (!graph) {
//         qDebug() << "Error: Q3DSurface pointer is null.";
//         delete[] rdmap;
//         return;
//     }

//     // 1. 清理旧系列 (如果图表已显示其他内容)
//     if (!graph->seriesList().isEmpty()) {
//         graph->removeSeries(graph->seriesList().first());
//     }

//     // 2. 数据代理和系列创建
//     QSurfaceDataProxy *dataProxy = new QSurfaceDataProxy();
//     QSurface3DSeries *series = new QSurface3DSeries(dataProxy);

//     // 3. 填充数据数组
//     QSurfaceDataArray *dataArray = new QSurfaceDataArray();

//     // --- 第一次循环：找到绝对最大功率值 ---
//     double absoluteMaxPower = 0.0;
//     for (int i = 0; i < RDMAP_SIZE; ++i) {
//         if ((double)rdmap[i] > absoluteMaxPower) {
//             absoluteMaxPower = (double)rdmap[i];
//         }
//     }

//     // 如果最大值仍是 0，无法进行归一化。
//     if (absoluteMaxPower == 0.0) {
//         qDebug() << "ERROR: Absolute max power is zero. Cannot display surface.";
//         delete[] rdmap;
//         return;
//     }


//     double calculatedMinNorm = std::numeric_limits<double>::max();
//     double calculatedMaxNorm = std::numeric_limits<double>::min();

//     const float Z_AXIS_SCALE_FACTOR = 100.0f; // 放大 Z 轴的视觉效果，让 1.78 dB 变得明显

//     // Range（Y轴）作为行，Doppler（X轴）作为列
//     for (int i = 0; i < RNG_NUM; ++i) { // Range Bins (Rows/Y-Axis)
//         QSurfaceDataRow *newRow = new QSurfaceDataRow(FFT_2D_SIZE);

//         for (int j = 0; j < FFT_2D_SIZE; ++j) { // Doppler Bins (Columns/X-Axis)
//             int index = i * FFT_2D_SIZE + j;
//             double powerValue = (double)rdmap[index];

//             double dbNormValue;
//             if (powerValue > 0) {
//                 // *** 归一化 dB 计算 ***
//                 // 最大值为 0 dB，其他为负值，这样能更好地突出目标
//                 dbNormValue = 10.0 * (std::log(powerValue / absoluteMaxPower) / std::log(10.0));
//             } else {
//                 dbNormValue = -120.0; // 设定一个合理的底噪声值
//             }

//             // 更新归一化后的 Min/Max 范围
//             if (dbNormValue > calculatedMaxNorm) calculatedMaxNorm = dbNormValue;
//             if (dbNormValue < calculatedMinNorm) calculatedMinNorm = dbNormValue;

//             // 填充数据项：同时应用 Z 轴放大因子
//             (*newRow)[j].setPosition({(float)j, (float)i, (float)dbNormValue * Z_AXIS_SCALE_FACTOR});
//         }
//         dataArray->append(newRow);
//     }

//     delete[] rdmap;

//     // 4. 设置数据代理 (保持不变)
//     dataProxy->resetArray(dataArray);

//     // 5. 应用动态范围到 Z轴 (Power) (!!! 关键修正：使用归一化范围并考虑放大因子 !!!)

//     // Z 轴的实际显示范围应该包括归一化后的数据范围，并乘上放大因子
//     double minDisplayZ = calculatedMinNorm * Z_AXIS_SCALE_FACTOR;
//     double maxDisplayZ = calculatedMaxNorm * Z_AXIS_SCALE_FACTOR;

//     // 设置 Z 轴 (功率) 范围
//     QValue3DAxis *zAxis = graph->axisZ();
//     zAxis->setRange(minDisplayZ, maxDisplayZ);
//     zAxis->setTitle("Normalized Power (dB)");
//     zAxis->setLabelFormat("%.1f dB"); // 标签仍显示 dB 值

//     qDebug() << "Data check: Min Norm dB =" << calculatedMinNorm << ", Max Norm dB =" << calculatedMaxNorm;
//     qDebug() << "Display Z Range (Scaled) =" << minDisplayZ << "to" << maxDisplayZ;

//     // 6. 设置颜色映射 (保持不变)
//     // ... (QLinearGradient and QImage texture creation remains the same) ...
//     QLinearGradient gradient(0, 0, 1, 100);
//     gradient.setColorAt(0.0, Qt::blue);
//     gradient.setColorAt(0.2, Qt::cyan);
//     gradient.setColorAt(0.5, Qt::green);
//     gradient.setColorAt(0.8, Qt::yellow);
//     gradient.setColorAt(1.0, Qt::red);
//     const int textureSize = 100;
//     QImage texture(1, textureSize, QImage::Format_RGB32);
//     QPainter painter(&texture);
//     painter.fillRect(texture.rect(), gradient);
//     series->setTexture(texture);
//     series->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);


//     // 7. 设置 X 和 Y 轴 (保持不变)
//     QValue3DAxis *xAxis = graph->axisX();
//     xAxis->setTitle(QString("Doppler/Velocity Bins (FFT_2D_SIZE=%1)").arg(FFT_2D_SIZE));
//     xAxis->setRange(0, FFT_2D_SIZE);

//     QValue3DAxis *yAxis = graph->axisY();
//     yAxis->setTitle(QString("Range Bins (RNG_NUM=%1)").arg(RNG_NUM));
//     yAxis->setRange(0, RNG_NUM);

//     // 8. 附加系列 (保持不变)
//     graph->addSeries(series);

//     // 9. 视图设置 (保持不变)
//     graph->setSelectionMode(QAbstract3DGraph::SelectionNone);
//     graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);

//     // 10. 强制设置一个俯视视角，更容易看到曲面
//     if (graph->activeInputHandler() == nullptr) {
//         // 如果没有，手动创建一个 Q3DInputHandler (这是默认的输入处理器)
//         Q3DInputHandler *inputHandler = new Q3DInputHandler(graph);
//         graph->setActiveInputHandler(inputHandler);


//     }

//     // 确保初始视角是用户友好的（如果之前被注释掉，请恢复）
//     // 这保证了曲面不会在视野外
//     graph->scene()->activeCamera()->setCameraPosition(0, 90, 0);

// }

float* uint16VectorToFloatPtr(const std::vector<uint16_t>& inputVector) {
    if (inputVector.empty()) {
        return nullptr;
    }

    qint64 numElements = inputVector.size();
    float* dataArray = nullptr;

    try {
        dataArray = new float[numElements];
    } catch (const std::bad_alloc& e) {
        qCritical() << "Error allocating memory for float array:" << e.what();
        return nullptr;
    }

    // 循环转换数据类型
    for (qint64 i = 0; i < numElements; ++i) {
        // 执行 uint16 到 float 的类型转换
        // 注意：uint16_t 是无符号的，这里是直接数值转换。
        // 如果需要归一化到 [-1, 1] 或 [0, 1]，请在此处添加逻辑。
        dataArray[i] = static_cast<float>(inputVector[i]);
    }

    // 调用者负责释放 dataArray
    // qDebug() << "Successfully converted" << numElements << "uint16 samples to float array.";
    return dataArray;
}


float* readBinToFloatPtr(const QString& filename) {
    // qDebug() << "start reading int16 to float conversion...";
    // QString filename = "C:/Users/JPN1WX/Desktop/QT/PointCloudUI/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/inputadc_int16.bin";

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Error opening file:" << filename << "-" << file.errorString();
        return nullptr;
    }

    qint64 fileSize = file.size();

    // --- 步骤 1: 读取原始 int16_t 数据 ---

    // 1.1 检查和计算 int16_t 元素的个数
    if (fileSize % sizeof(qint16) != 0) {
        qWarning() << "Warning: File size is not a multiple of int16_t size (2 bytes). Data may be incomplete.";
    }
    qint64 numInt16s = fileSize / sizeof(qint16);

    // 1.2 临时分配内存用于存储原始 int16_t 数据
    qint16* rawInt16Array = nullptr;
    try {
        rawInt16Array = new qint16[numInt16s];
    } catch (const std::bad_alloc& e) {
        qCritical() << "Error allocating memory for int16 array:" << e.what();
        file.close();
        return nullptr;
    }

    // 1.3 读取原始数据到 int16 数组
    qint64 bytesRead = file.read(
        reinterpret_cast<char*>(rawInt16Array),
        fileSize
        );
    file.close();

    if (bytesRead != fileSize) {
        qCritical() << "Error reading file: Expected" << fileSize << "bytes, read" << bytesRead;
        delete[] rawInt16Array;
        return nullptr;
    }

    // --- 步骤 2: 转换并分配最终 float* 内存 ---

    // 2.1 分配最终 float 数组的内存
    float* dataArray = nullptr;
    try {
        // numFloats 应该等于 numInt16s
        dataArray = new float[numInt16s];
    } catch (const std::bad_alloc& e) {
        qCritical() << "Error allocating memory for float array:" << e.what();
        delete[] rawInt16Array; // 必须释放
        return nullptr;
    }

    // 2.2 循环转换数据类型
    for (qint64 i = 0; i < numInt16s; ++i) {
        // 执行 int16 到 float 的类型转换
        dataArray[i] = static_cast<float>(rawInt16Array[i]);
    }

    // 2.3 释放临时分配的 int16 数组
    delete[] rawInt16Array;

    // qDebug() << "Successfully read and converted" << numInt16s << "int16 samples to float array.";
    return dataArray;
}


float* int16VectorToFloatPtr(const std::vector<int16_t>& inputVector) {
    if (inputVector.empty()) {
        // qWarning() << "Input vector for int16VectorToFloatPtr is empty.";
        return nullptr;
    }

    qint64 numElements = inputVector.size();
    float* dataArray = nullptr;

    try {
        // 动态分配内存用于存储 float 数组
        dataArray = new float[numElements];
    } catch (const std::bad_alloc& e) {
        qCritical() << "Error allocating memory for float array in int16VectorToFloatPtr:" << e.what();
        return nullptr;
    }

    // 循环转换数据类型
    for (qint64 i = 0; i < numElements; ++i) {
        // 执行 qint16 到 float 的类型转换
        // qint16 是有符号的，static_cast<float> 会正确地保留其符号和数值。
        // 如果需要归一化到 [-1, 1] 或 [0, 1] 等范围，请在此处添加逻辑。
        dataArray[i] = static_cast<float>(inputVector[i]);
    }

    // 调用者负责使用 delete[] dataArray 释放返回的 dataArray 指针
    // qDebug() << "Successfully converted" << numElements << "qint16 samples to float array.";
    return dataArray;
}


std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> setupRangeDopplerPlotnew(QCustomPlot *customPlot,const QString& filename) {



    // 1. 确保 customPlot 非空
    if (!customPlot) {
        qDebug() << "Error: QCustomPlot pointer is null.";
        return nullptr;
    }


    // 2. 清理旧图层和数据 (重要，防止重复积累)
    customPlot->clearPlottables();
    customPlot->clearGraphs();

    // ... (DLL加载和数据获取逻辑，这里简化)
    // 假设 rdmap 数据已经成功获取并填充
    // qDebug()<<"start";

    std::vector<uint16_t> adc_data_uint16 = TransferOneFrame8_8RadarADC(filename);
    std::vector<int16_t> result_vector = process_matlab_typecast_and_bitshift(adc_data_uint16);


    if (adc_data_uint16.empty()) {
        qCritical() << "Processing failed or no data returned.";
        return nullptr;
    }

    // float* timedata = readBinToFloatPtr(filename);
    // float* timedata = uint16VectorToFloatPtr(adc_data_uint16);

    // 🌟 修正 1: 使用 std::unique_ptr 管理 timedata (假设它由 new[] 分配)
    std::unique_ptr<float[]> timedata_ptr(int16VectorToFloatPtr(result_vector));
    if (!timedata_ptr) return nullptr;
    float* timedata = timedata_ptr.get();
    // std::cout<<"hello"<<std::endl;

    VehiclesInfo_t vehInfo ;
    vehInfo.speed = 0.0f;       // 例如，设置为 10.5 m/s
    vehInfo.yawrate = 0.0f;      // 例如，设置为 0.1 rad/s
    vehInfo.acceleration = 0.0f; // 例如，设置为 1.0 m/s^2

    // 初始化枚举成员
    vehInfo.gearInfo = D;        // 例如，设置为前进挡 (D)

    RadarInfo_t radarInfo;
    radarInfo.radarId = Front;
    radarInfo.frameCount = 1; // 例如，设置为帧计数 100

    // 初始化 double 数组 (timeStamp[100])
    // 建议至少初始化第一个元素，防止 DLL 内部访问到未初始化数据
    for (int i = 0; i < 1; ++i) {
        radarInfo.timeStamp[i] = 0.0; // 全部初始化为 0.0
    }

    // 初始化嵌套结构体 MountenInfo_t
    radarInfo.mountenInfo.mountenR = 0.0f;
    radarInfo.mountenInfo.mountenYaw = 0.0f;
    radarInfo.mountenInfo.mountenPitch=0.0f;
    radarInfo.mountenInfo.mountenRoll=0.0f;

    auto pc_deleter = [](PointCloud2_t* p) {
        // 🚨 必须确保在这里清理 PointCloud2_t 内部的所有数组
        // 例如：delete[] p->x; delete[] p->y; ...
        delete p;
    };

    std::unique_ptr<PointCloud2_t, decltype(pc_deleter)> pointCloud_ptr(new (std::nothrow) PointCloud2_t{}, pc_deleter);
    if (!pointCloud_ptr) { qCritical() << "Error: Failed to allocate memory for PointCloud2_t."; return nullptr; }

    PointCloud2_t* pointCloud = pointCloud_ptr.get();

    // PointCloud2_t* pointCloud = new PointCloud2_t;
    if (pointCloud == nullptr) {
        qCritical() << "Error: Failed to allocate memory for PointCloud2_t.";
        // 这里应该加入资源清理和错误处理
        return nullptr;
    }
    // uint32_t rdmap[RDMAP_SIZE] = {0};
    // float* rdmap = new float[RDMAP_SIZE];
    // 🌟 修正 2: 使用 std::unique_ptr 管理 rdmap
    std::unique_ptr<float[]> rdmap_ptr(new (std::nothrow) float[RDMAP_SIZE]);
    if (!rdmap_ptr) { qCritical() << "Error: Failed to allocate memory for rdmap."; return nullptr; }
    float* rdmap = rdmap_ptr.get();
    // std::cout<<"heihei"<<std::endl;
    RspProssingFunc rspProssingPtr = RspProcessor::getRspProssingFunction();
    if (rspProssingPtr) {
    try {

        rspProssingPtr(timedata, &vehInfo, &radarInfo, pointCloud, rdmap);
    } catch (const std::exception& e) {
        qCritical() << "Error: rspProssing (DSP/DLL) crashed with exception:" << e.what();
        return nullptr; // 发生错误，安全退出
    } catch (...) {
        qCritical() << "Error: rspProssing (DSP/DLL) crashed due to unknown fatal error.";
        return nullptr; // 发生未知错误，安全退出
    }
    }else{
        qCritical() << "Error: rspProssing function failed to load or resolve.";
        return nullptr;

    }
    // std::cout<<"noheihei"<<std::endl;
    // qDebug()<<pointCloud->num;
    // printPointCloud2(pointCloud);

    QFileInfo fileInfo(filename);
    QString csvPath =
        fileInfo.absolutePath() + "/" +
        fileInfo.baseName() + ".csv";
    bool success = writePointCloudToCsv(*pointCloud, csvPath.toStdString());
    // bool success = writePointCloudToCsv(*pointCloud, fileInfo.baseName().toStdString()+".csv");
    if (success) {
        // std::cout << "Data successfully written to xx.csv" << std::endl;
    } else {
        std::cerr << "Failed to write data to xx.csv" << std::endl;
    }

    updateRangeDopplerPlot(customPlot,rdmap);


    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> radar8x8pointdata = convertPointCloud2ToPCL(*pointCloud,0);
    // delete[] rdmap;
    // delete[] timedata;
    // delete pointCloud;
    // 注意：不再需要 delete customPlot，因为它是由 MainWindow 拥有的
    return radar8x8pointdata;
}

void printPointCloud2(const PointCloud2_t* ptPointCloud)
{
    if (ptPointCloud == NULL) {
        printf("错误：点云结构体指针为空。\n");
        return;
    }

    uint16_t count = ptPointCloud->num;
    printf("--- Point data starts ---\n");
    printf("targets total: %u\n", count);

    if (count == 0) {
        printf("no target\n");
        printf("--- data end ---\n");
        return;
    }

    // 限制打印的点数，以防 MAX_POINT_NUM 过大导致打印时间过长
    // 实际打印的点数取 count 和 MAX_POINT_NUM 中的较小值
    uint16_t num_to_print = (count < MAX_POINT_NUM) ? count : MAX_POINT_NUM;

    // 打印标题行
    printf("\n%4s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s | %8s\n",
           "Idx", "Range(m)", "Vel(m/s)", "Azimuth", "Elevat.", "RCS", "SNR", "R.Gate", "V.Gate", "Exist%");
    printf("----------------------------------------------------------------------------------------------------\n");

    // 循环打印每个点的数据
    for (uint16_t i = 0; i < num_to_print; i++)
    {
        // 打印核心测量数据
        printf("%4u | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f | %8u | %8u | %8u%%\n",
               i,
               ptPointCloud->range[i],
               ptPointCloud->vel[i],
               ptPointCloud->azimuth[i],
               ptPointCloud->elevation[i],
               ptPointCloud->rcs[i],
               ptPointCloud->snr[i],
               ptPointCloud->rangeGate[i],
               ptPointCloud->velGate[i],
               ptPointCloud->existProb[i]
               );

        // 可以在这里选择性地打印方差和质量信息
        /*
        printf("   -> 质量/状态: A_Q:%u E_Q:%u DV_Q:%u Status:%u\n",
               ptPointCloud->aziQly[i],
               ptPointCloud->eleQly[i],
               ptPointCloud->dvQly[i],
               ptPointCloud->measStatus[i]);
        printf("   -> 方差: R_Var:%.4f V_Var:%.4f A_Var:%.4f E_Var:%.4f\n",
               ptPointCloud->rangVar[i],
               ptPointCloud->velVar[i],
               ptPointCloud->azimuthVar[i],
               ptPointCloud->elevationVar[i]);
        */
    }

    printf("--- end ---\n");
}
bool writePointCloudToCsv(const PointCloud2_t& data, const std::string& filenameqt){
    std::wstring filename = utf8_to_wstring(filenameqt);
    // 1. 打开文件流
    std::ofstream outputFile(filename);

    // 检查文件是否成功打开
    if (!outputFile.is_open()) {
        std::cerr << "Error: Failed to open file "  << std::endl;
        return false;
    }

    // 设置浮点数的精度（可选，让输出更整洁）
    outputFile << std::fixed << std::setprecision(4);

    // 2. 写入 CSV 表头（Header）
    outputFile << "num,aziQly,eleQly,dvQly,measStatus,existProb,crossPathPowerRatio,";
    outputFile << "velGate,rangeGate,idxLocPeer,";
    outputFile << "range,rangVar,vel,velVar,rvCov,azimuth,azimuthVar,elevation,elevationVar,";
    outputFile << "rcs,snr,power,x,y,z,detValid\n";

    // 3. 遍历所有有效的点 (data.num) 并写入数据行
    // 注意：我们将 uint8_t 强制转换为 int 写入，以避免将其误识别为字符
    for (int i = 0; i < data.num; ++i)
    {
        // 确保索引在 MAX_POINT_NUM 范围内，尽管 data.num 应该保证这一点
        if (i >= MAX_POINT_NUM) break;

        // 写入总点数 (每行重复，用于记录)
        outputFile << i+1 << ",";

        // 写入 uint8_t 和 uint16_t 类型的属性
        outputFile << (int)data.aziQly[i] << ",";
        outputFile << (int)data.eleQly[i] << ",";
        outputFile << (int)data.dvQly[i] << ",";
        outputFile << (int)data.measStatus[i] << ",";
        outputFile << (int)data.existProb[i] << ",";
        outputFile << (int)data.crossPathPowerRatio[i] << ",";
        outputFile << data.velGate[i] << ",";
        outputFile << data.rangeGate[i] << ",";
        outputFile << data.idxLocPeer[i] << ",";

        // 写入 float 类型的属性
        outputFile << data.range[i] << ",";
        outputFile << data.rangVar[i] << ",";
        outputFile << data.vel[i] << ",";
        outputFile << data.velVar[i] << ",";
        outputFile << data.rvCov[i] << ",";
        outputFile << data.azimuth[i] << ",";
        outputFile << data.azimuthVar[i] << ",";
        outputFile << data.elevation[i] << ",";
        outputFile << data.elevationVar[i] << ",";
        outputFile << data.rcs[i] << ",";
        outputFile << data.snr[i] << ",";
        outputFile << data.power[i] << ",";
        outputFile << data.x[i] << ",";
        outputFile << data.y[i] << ",";
        outputFile << data.z[i] << ",";
        outputFile << static_cast<int>(data.detValid[i]);// 最后一个字段不带逗号
        // 换行，开始下一行数据
        outputFile << "\n";

    }

    // 4. 关闭文件流
    outputFile.close();

    // std::cout << "Successfully wrote " << data.num << " points to " << filename << std::endl;
    return true;
}

std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> convertPointCloud2ToPCL(const PointCloud2_t& sourceData,int currentFrameIndex)
{
    // 1. 创建目标 PCL 点云对象
    std::shared_ptr<pcl::PointCloud<PointXYZRGBWithProperties>> cloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();

    // 2. 预分配内存和设置点云大小
    // 仅使用 num 作为有效的点数
    size_t numPoints = sourceData.num;

    // 确保点数在 MAX_POINT_NUM 范围内，防止越界
    if (numPoints > MAX_POINT_NUM) {
        numPoints = MAX_POINT_NUM;
    }

    cloud->points.resize(numPoints);
    cloud->width = numPoints;
    cloud->height = 1; // 无序点云

    // 3. 遍历原始数组数据并进行转换/映射
    for (size_t i = 0; i < numPoints; ++i)
    {
        // 获取目标点引用
        PointXYZRGBWithProperties& dst = cloud->points[i];

        // ----------------------------------------
        // A. 基础坐标 (XYZ)
        // ----------------------------------------
        dst.x = sourceData.x[i];
        dst.y = sourceData.y[i];
        dst.z = sourceData.z[i];

        // ----------------------------------------
        // B. 颜色 (RGB)
        // ----------------------------------------
        // 🚨 颜色映射：PointCloud2_t 原始结构体中没有 R, G, B 字段。
        // 这里需要根据您的业务逻辑，将原始属性 (如 rcs, snr) 映射到 R, G, B。
        // 假设此处为固定颜色，或者基于某个属性进行简单映射（例如：基于SNR）

        // 示例：将 SNR 映射到 R, G, B，此处简化为灰色
        uint8_t color_val = static_cast<uint8_t>(std::min(255.0f, sourceData.snr[i] * 10.0f));
        dst.r = color_val;
        dst.g = color_val;
        dst.b = color_val;

        // ----------------------------------------
        // C. 自定义属性 (映射与计算)
        // ----------------------------------------

        // 直接映射
        dst.range = sourceData.range[i];
        dst.azimuthAng = sourceData.azimuth[i];
        dst.eleAng = sourceData.elevation[i];

        // 映射并可能需要单位转换 (假设 rcs/snr/power 已经是 dB)
        dst.rcsdB = sourceData.rcs[i];
        dst.SNRdB = sourceData.snr[i];
        dst.powerdB = sourceData.power[i];

        // 映射和计算
        // 1. dopplerSpeed 映射到 vel
        dst.dopplerSpeed = sourceData.vel[i];

        // 2. radVelAbs (绝对值)
        dst.radVelAbs = sourceData.vel[i];
        // dst.radVelAbs = std::abs(sourceData.vel[i]);

        // 3. Q_azi / Q_ele (质量)
        // 假设映射到归一化后的 aziQly/eleQly (0-255 -> 0.0-1.0)
        dst.Q_azi = (float)sourceData.aziQly[i] / 255.0f;
        dst.Q_ele = (float)sourceData.eleQly[i] / 255.0f;

        // 4. detValid (检测有效性)
        // 假设 measStatus=1 表示有效 (1.0)，其他表示无效 (0.0)
        dst.detValid = (sourceData.detValid[i] != 0) ? 1.0f : 0.0f;

        // 5. 额外的 originalIndex
        dst.originalIndex = (int)i;
        dst.frameIndex = currentFrameIndex;
    }

    return cloud;
}

std::vector<pointandtimedata8x8> GetRadar8T8Datafrommf4(const QString& filename,const QString& savePath,double start,double end){

    std::vector<pointandtimedata8x8> result;


    QFileInfo info(savePath);
    QString CSVPath = info.absolutePath() + "/" + info.completeBaseName() + ".csv";
    QString BINFILE = info.absolutePath() + "/" + info.completeBaseName() + ".bin";
    writePointCloudCsvHeader(CSVPath.toStdString());


    // 1. 读取 MF4 所有帧
    std::vector<TimestampedData> collected_data =readallmdftobin(filename.toStdString(), start, end);
    std::vector<pointandtimedata> alldata;
    const int DIM8T8_1 = 512;
    const int DIM8T8_2 = 1024;
    const int DIM8T8_3 = 8;
    const size_t total_size = DIM8T8_1 * DIM8T8_2 * DIM8T8_3;
    int frameIndex = 0;

    for (const auto& data_point : collected_data){
        // ---------------------
        // Step 1. 解码 Raw12 → uint16
        // ---------------------
        size_t offset = 0;
        size_t frame_size = data_point.data.size();
        auto raw12_data = extractRaw12Frames8x8(data_point.data, offset, frame_size);
        auto pixels = decodeRaw12(raw12_data);
        std::vector<int16_t> result_vector = process_matlab_typecast_and_bitshift(pixels);

        if (pixels.size() != total_size) {
            qWarning() << "Frame" << frameIndex << "data format error";
            continue;
        }

        float* timedata = int16VectorToFloatPtr(result_vector);

        VehiclesInfo_t vehInfo ;
        vehInfo.speed = 0.0f;       // 例如，设置为 10.5 m/s
        vehInfo.yawrate = 0.0f;      // 例如，设置为 0.1 rad/s
        vehInfo.acceleration = 0.0f; // 例如，设置为 1.0 m/s^2

        // 初始化枚举成员
        vehInfo.gearInfo = D;        // 例如，设置为前进挡 (D)

        RadarInfo_t radarInfo;
        radarInfo.radarId = Front;
        radarInfo.frameCount = frameIndex; // 例如，设置为帧计数 100

        // 初始化 double 数组 (timeStamp[100])
        // 建议至少初始化第一个元素，防止 DLL 内部访问到未初始化数据
        for (int i = 0; i < 1; ++i) {
            radarInfo.timeStamp[i] = 0.0; // 全部初始化为 0.0
        }

        // 初始化嵌套结构体 MountenInfo_t
        radarInfo.mountenInfo.mountenR = 0.0f;
        radarInfo.mountenInfo.mountenYaw = 0.0f;
        radarInfo.mountenInfo.mountenPitch=0.0f;
        radarInfo.mountenInfo.mountenRoll=0.0f;

        // ---------------------
        // Step 3. 分配 pointCloud 和 rdmap
        // ---------------------
        auto pointCloud = std::make_unique<PointCloud2_t>();
        std::unique_ptr<float[]> rdmap(new float[RDMAP_SIZE]);
        RspProssingFunc rspProssingPtr = RspProcessor::getRspProssingFunction();
        rspProssingPtr(timedata, &vehInfo, &radarInfo, pointCloud.get(), rdmap.get());

        delete[] timedata;

        pointandtimedata8x8 frame;
        frame.indexno = frameIndex;
        frame.start_time_ns = data_point.start_time_ns;
        frame.timestamp_ns  = data_point.timestamp_ns;
        // ---------------------
        // Step 4. 保存 CSV（可选）
        // ---------------------
        // writePointCloudToCsv_Append(frameIndex,
        //                             data_point.start_time_ns,
        //                             *pointCloud,
        //                             CSVPath.toStdString());

        // ---------------------
        // Step 5. 转成 PCL 点云
        // ---------------------
        frame.Radar8T8Point.cloud = convertPointCloud2ToPCL(*pointCloud,frameIndex);

        frame.Radar8T8Point.rdmap.assign(
            rdmap.get(), rdmap.get() + RDMAP_SIZE);
        // 加入结果
        result.push_back(std::move(frame));
        frameIndex++;


    }
    Save8T8VectorToBin(BINFILE.toStdString(),result) ;
    return result;
}

std::vector<pointandtimedata8x8> GetRadar8T8Datafrommf4byfiles(const QString& filename,const QString& savePath,double start,double end){

    std::vector<pointandtimedata8x8> result;


    QFileInfo info(savePath);
    QString CSVPath = info.absolutePath() + "/" + info.completeBaseName() + ".csv";
    QString BINFILE = info.absolutePath() + "/" + info.completeBaseName() + ".bin";
    // ⚡ 关键修正 1: 调用初始化函数，写入占位符

    QString tmpDir = QDir::tempPath() + "/pointcloud_temp";
    QDir().mkpath(tmpDir);

    QString tmpBINFILE = tmpDir + "/" + QUuid::createUuid().toString() + ".bin";
    QString tmpCSVPath = tmpDir + "/" + QUuid::createUuid().toString() + ".csv";
    QString tempBinOutputDir = tmpDir + "/temp_radar_frames";
    QDir().mkpath(tempBinOutputDir);


    try {
        Init8T8BinFile(tmpBINFILE.toStdString());
    } catch (const std::exception& e) {
        qCritical() << "Error initializing BIN file:" << e.what();
        return result;
    }

    writePointCloudCsvHeader(tmpCSVPath.toLocal8Bit().toStdString());


    // 1. 读取 MF4 所有帧
    QElapsedTimer tReadMF4;
    tReadMF4.start();
    std::vector<FrameMetadata> collected_frames_metadata =readallmdftobin_to_files(filename.toStdString(), tempBinOutputDir.toStdString(), start, end);

    qDebug() << "[PERF] MF4 read:" << tReadMF4.elapsed() << "ms";

    std::vector<pointandtimedata> alldata;
    const int DIM8T8_1 = 512;
    const int DIM8T8_2 = 1024;
    const int DIM8T8_3 = 8;
    const size_t total_size = DIM8T8_1 * DIM8T8_2 * DIM8T8_3;
    int frameIndex = 0;
    int savedFrameCount = 0;
    for (const auto& frame_info : collected_frames_metadata){

        // if (frame_info.frame_index == 611 ||frame_info.frame_index == 698||frame_info.frame_index == 790||frame_info.frame_index == 1342||frame_info.frame_index == 2669||frame_info.frame_index == 2887||frame_info.frame_index == 2954||frame_info.frame_index == 4784||frame_info.frame_index == 4952||frame_info.frame_index == 5525||frame_info.frame_index == 5557||frame_info.frame_index == 5571||frame_info.frame_index == 5685) {
        //     qWarning() << "Skipping frame processing as requested for frame index:" << frameIndex;
        //     frameIndex++;
        //     continue;
        // }
        QElapsedTimer tBinRead;
        tBinRead.start();
        std::vector<uint8_t> raw_video_bytes;
        QFile frameFile(QString::fromStdString(frame_info.bin_filepath));
        if (frameFile.open(QIODevice::ReadOnly)) {
            QByteArray byteArray = frameFile.readAll(); // 读取到 QByteArray
            // 将 QByteArray 转换为 std::vector<uint8_t>
            raw_video_bytes.assign(reinterpret_cast<const uint8_t*>(byteArray.constData()),
                                   reinterpret_cast<const uint8_t*>(byteArray.constData()) + byteArray.size());
            frameFile.close();
        } else {
            qCritical() << "Error: Failed to read frame data from file:" << QString::fromStdString(frame_info.bin_filepath);
            frameIndex++; // 即使失败，也更新索引以继续
            continue;
        }

        qDebug() << "[PERF] Frame" << frameIndex << "Readbin:" << tBinRead.elapsed() << "ms";
        // ---------------------
        // Step 1. 解码 Raw12 → uint16
        // ---------------------
        QElapsedTimer tDecode;
        tDecode.start();

        size_t offset = 0;
        size_t frame_size = raw_video_bytes.size();
        auto raw12_data = extractRaw12Frames8x8(raw_video_bytes, offset, frame_size);
        auto pixels = decodeRaw12(raw12_data);
        std::vector<int16_t> result_vector = process_matlab_typecast_and_bitshift(pixels);

        qDebug() << "[PERF] Frame" << frameIndex << "Raw12 tDecode:" << tDecode.elapsed() << "ms";

        if (pixels.size() != total_size) {
            qWarning() << "Frame" << frameIndex << "data format error";
            continue;
        }

        float* timedata = int16VectorToFloatPtr(result_vector);

        VehiclesInfo_t vehInfo ;
        vehInfo.speed = 0.0f;       // 例如，设置为 10.5 m/s
        vehInfo.yawrate = 0.0f;      // 例如，设置为 0.1 rad/s
        vehInfo.acceleration = 0.0f; // 例如，设置为 1.0 m/s^2

        // 初始化枚举成员
        vehInfo.gearInfo = D;        // 例如，设置为前进挡 (D)

        RadarInfo_t radarInfo;
        radarInfo.radarId = Front;
        radarInfo.frameCount = frameIndex; // 例如，设置为帧计数 100

        // 初始化 double 数组 (timeStamp[100])
        // 建议至少初始化第一个元素，防止 DLL 内部访问到未初始化数据
        for (int i = 0; i < 1; ++i) {
            radarInfo.timeStamp[i] = 0.0; // 全部初始化为 0.0
        }

        // 初始化嵌套结构体 MountenInfo_t
        radarInfo.mountenInfo.mountenR = 0.0f;
        radarInfo.mountenInfo.mountenYaw = 0.0f;
        radarInfo.mountenInfo.mountenPitch=0.0f;
        radarInfo.mountenInfo.mountenRoll=0.0f;

        // ---------------------
        // Step 3. 分配 pointCloud 和 rdmap
        // ---------------------
        auto pointCloud = std::make_unique<PointCloud2_t>();
        std::unique_ptr<float[]> rdmap(new float[RDMAP_SIZE]);
        QElapsedTimer tDSP;
        tDSP.start();

        // rspProssing(timedata, &vehInfo, &radarInfo, pointCloud.get(), rdmap.get());
        RspProssingFunc rspProssingPtr = RspProcessor::getRspProssingFunction();
        if (rspProssingPtr) {
        try {
            rspProssingPtr(timedata, &vehInfo, &radarInfo, pointCloud.get(), rdmap.get());
        } catch (const std::exception& e) {
            qCritical() << "Frame" << frameIndex << "Runtime error in rspProssing (DSP/DLL):" << e.what() << ". Skipping frame.";
            delete[] timedata; // 清理资源
            continue; // 跳过当前帧
        } catch (...) {
            qCritical() << "Frame" << frameIndex << "Unknown fatal error in rspProssing. Skipping frame.";
            delete[] timedata; // 清理资源
            continue; // 跳过当前帧
        }
        } else {
            qCritical() << "Error: rspProssing function failed to load or resolve.";

        }
        qDebug() << "[PERF] Frame" << frameIndex << "DSP:" << tDSP.elapsed() << "ms";
        delete[] timedata;

        pointandtimedata8x8 frame;
        frame.indexno = frameIndex;
        frame.start_time_ns = frame_info.file_start_time_ns;
        frame.timestamp_ns  = frame_info.relative_timestamp_ns;
        // ---------------------
        // Step 4. 保存 CSV（可选）
        // ---------------------
        // writePointCloudToCsv_Append(frameIndex,
        //                             data_point.start_time_ns,
        //                             *pointCloud,
        //                             CSVPath.toStdString());

        // ---------------------
        // Step 5. 转成 PCL 点云
        // ---------------------
        QElapsedTimer tPCL;
        tPCL.start();

        frame.Radar8T8Point.cloud = convertPointCloud2ToPCL(*pointCloud,frameIndex);

        qDebug() << "[PERF] Frame" << frameIndex << "PCL:" << tPCL.elapsed() << "ms";
        frame.Radar8T8Point.rdmap.assign(
            rdmap.get(), rdmap.get() + RDMAP_SIZE);
        // 加入结果
        // ⚡ 关键修正 2B: 逐帧存储到 BIN 文件 (确保成功后才更新索引)+
        QElapsedTimer tSave;
        tSave.start();

        try {
            Save8T8FrameToBin(tmpBINFILE.toStdString(), frame);
            savedFrameCount++;
            qDebug() << "Successfully saved frame:" << frame_info.frame_index << "to BIN file:" << BINFILE;

        } catch (const std::exception& e) {
            qCritical() << "Error saving frame" << frameIndex << "to BIN file:" << e.what();
            // 存储失败，不增加 frameIndex，跳过当前帧
            continue;
        }
        qDebug() << "[PERF] Frame" << frameIndex << "saved:" << tSave.elapsed() << "ms";
        result.push_back(std::move(frame));

        frameIndex++;




    }

    // Save8T8VectorToBin(BINFILE.toStdString(),result) ;
    try {
        Finalize8T8BinFile(tmpBINFILE.toStdString(), static_cast<uint64_t>(savedFrameCount));
    } catch (const std::exception& e) {
        qCritical() << "Error finalizing BIN file:" << e.what();
    }

    if (QFile::exists(BINFILE)) QFile::remove(BINFILE);
    if (!QFile::copy(tmpBINFILE, BINFILE))
        qCritical() << "Failed to copy BIN file to final path:" << BINFILE;

    if (QFile::exists(CSVPath)) QFile::remove(CSVPath);
    if (!QFile::copy(tmpCSVPath, CSVPath))
        qCritical() << "Failed to copy CSV file to final path:" << CSVPath;

    return result;
}

bool writePointCloudCsvHeader(const std::string& filename_qt)
{
    std::wstring filename = utf8_to_wstring(filename_qt);
    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) return false;

    outputFile << "frameIndex,timestamp,pointIndex,";
    outputFile << "aziQly,eleQly,dvQly,measStatus,existProb,crossPathPowerRatio,";
    outputFile << "velGate,rangeGate,idxLocPeer,";
    outputFile << "range,rangVar,vel,velVar,rvCov,azimuth,azimuthVar,elevation,elevationVar,";
    outputFile << "rcs,snr,power,x,y,z\n";

    outputFile.close();
    return true;
}

bool writePointCloudToCsv_Append(int frameIndex, double timestamp,
                                 const PointCloud2_t& data,
                                 const std::string& filename)
{
    std::ofstream outputFile(filename, std::ios::app);
    if (!outputFile.is_open()) return false;

    outputFile << std::fixed << std::setprecision(4);

    for (int i = 0; i < data.num; ++i)
    {
        outputFile << frameIndex << "," << timestamp << "," << i << ",";

        outputFile << (int)data.aziQly[i] << ",";
        outputFile << (int)data.eleQly[i] << ",";
        outputFile << (int)data.dvQly[i] << ",";
        outputFile << (int)data.measStatus[i] << ",";
        outputFile << (int)data.existProb[i] << ",";
        outputFile << (int)data.crossPathPowerRatio[i] << ",";
        outputFile << data.velGate[i] << ",";
        outputFile << data.rangeGate[i] << ",";
        outputFile << data.idxLocPeer[i] << ",";

        outputFile << data.range[i] << ",";
        outputFile << data.rangVar[i] << ",";
        outputFile << data.vel[i] << ",";
        outputFile << data.velVar[i] << ",";
        outputFile << data.rvCov[i] << ",";
        outputFile << data.azimuth[i] << ",";
        outputFile << data.azimuthVar[i] << ",";
        outputFile << data.elevation[i] << ",";
        outputFile << data.elevationVar[i] << ",";
        outputFile << data.rcs[i] << ",";
        outputFile << data.snr[i] << ",";
        outputFile << data.power[i] << ",";
        outputFile << data.x[i] << ",";
        outputFile << data.y[i] << ",";
        outputFile << data.z[i] << "\n";
    }

    return true;
}

// void updateRangeDopplerPlot(QCustomPlot *customPlot, const float *rdmap)
// {
//     if (!customPlot || !rdmap) return;

//     // --- 1️⃣ 获取或创建 colorMap ---
//     QCPColorMap *colorMap = nullptr;
//     if (customPlot->plottableCount() > 0)
//         colorMap = qobject_cast<QCPColorMap*>(customPlot->plottable(0));

//     if (!colorMap)
//     {
//         // 关联主轴
//         colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);

//         // 创建右侧 colorScale（只创建一次）
//         QCPColorScale *colorScale = nullptr;
//         if (customPlot->plotLayout()->elementCount() > 1)
//             colorScale = qobject_cast<QCPColorScale*>(customPlot->plotLayout()->element(0, 1));
//         if (!colorScale)
//         {
//             colorScale = new QCPColorScale(customPlot);
//             customPlot->plotLayout()->addElement(0, 1, colorScale); // 右侧颜色条
//             colorScale->setType(QCPAxis::atRight);
//         }
//         colorMap->setColorScale(colorScale);
//     }

//     // --- 2️⃣ 设置绿色→黄色渐变 ---
//     QCPColorGradient greenYellowGrad;
//     greenYellowGrad.clearColorStops();
//     greenYellowGrad.setColorStopAt(0.0, QColor(0, 128, 0));       // 深绿色
//     greenYellowGrad.setColorStopAt(0.5, QColor(128, 255, 0));     // 黄绿色
//     greenYellowGrad.setColorStopAt(1.0, QColor(255, 255, 0));     // 黄色
//     colorMap->setGradient(greenYellowGrad);
//     colorMap->setInterpolate(true); // 平滑颜色过渡

//     // --- 3️⃣ 填充数据并计算 min/max ---
//     const int nx = FFT_2D_SIZE;
//     const int ny = RNG_NUM;
//     colorMap->data()->setSize(nx, ny);

//     double dataMin = std::numeric_limits<double>::max();
//     double dataMax = std::numeric_limits<double>::lowest();

//     for (int i = 0; i < ny; ++i)
//     {
//         for (int j = 0; j < nx; ++j)
//         {
//             int idx = i * nx + j;
//             double val = rdmap[idx] > 1e-12 ? 10.0 * log10(rdmap[idx]) : 77.0;
//             colorMap->data()->setCell(j, i, val);

//             dataMin = std::min(dataMin, val);
//             dataMax = std::max(dataMax, val);
//         }
//     }

//     // --- 4️⃣ 设置 Z 轴显示范围（可选压缩动态范围） ---
//     double dynamicRange = 60.0;
//     double zMax = dataMax;
//     double zMin = std::max(dataMin, zMax - dynamicRange); // 保证动态范围
//     colorMap->setDataRange(QCPRange(zMin, zMax));

//     // --- 5️⃣ 设置坐标轴范围 ---
//     colorMap->data()->setKeyRange(QCPRange(0, nx));
//     colorMap->data()->setValueRange(QCPRange(0, ny));
//     customPlot->xAxis->setRange(0, nx);
//     customPlot->yAxis->setRange(0, ny);
//     customPlot->axisRect()->setupFullAxesBox(true);

//     // --- 6️⃣ 标签与刷新 ---
//     customPlot->xAxis->setLabel("Velocity Bins");
//     customPlot->yAxis->setLabel("Range Bins");
//     colorMap->colorScale()->axis()->setLabel("Amplitude (dB)");

//     customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//     customPlot->replot();
// }

void updateRangeDopplerPlot(QCustomPlot *customPlot, const float *rdmap)
{
    if (!customPlot || !rdmap) return;

    const int nx = FFT_2D_SIZE; // 512
    const int ny = RNG_NUM;    // 1024

    // --- 1️⃣ 获取或创建 colorMap (复用模式，不 clear) ---
    QCPColorMap *colorMap = qobject_cast<QCPColorMap*>(customPlot->plottable(0));

    if (!colorMap) {
        colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);

        // 第一次创建时才配置布局
        QCPColorScale *colorScale = new QCPColorScale(customPlot);
        customPlot->plotLayout()->addElement(0, 1, colorScale);
        colorScale->setType(QCPAxis::atRight);
        colorMap->setColorScale(colorScale);

        QCPColorGradient grad;
        grad.setColorStopAt(0.0, QColor(0, 128, 0));
        grad.setColorStopAt(0.5, QColor(128, 255, 0));
        grad.setColorStopAt(1.0, QColor(255, 255, 0));
        colorMap->setGradient(grad);

        colorMap->data()->setSize(nx, ny);
        colorMap->data()->setKeyRange(QCPRange(0, nx));
        colorMap->data()->setValueRange(QCPRange(0, ny));
    }

    // --- 2️⃣ 填充数据 (优化后的循环) ---
    double dataMax = -100.0;
    double dataMin = 100.0;

    // 获取数据对象的指针，虽然不能直接拿 double*，但可以预取 data 对象减少调用开销
    QCPColorMapData *mapData = colorMap->data();

    for (int i = 0; i < ny; ++i) {
        for (int j = 0; j < nx; ++j) {
            float rawVal = rdmap[i * nx + j]; // 原始数据

            // 修正拼写错误并进行对数转换
            double val = (rawVal > 1e-12f) ? 10.0 * log10(static_cast<double>(rawVal)) : -77.0;

            // 使用 setCell 写入
            mapData->setCell(j, i, val);

            if (val > dataMax) dataMax = val;
            if (val < dataMin) dataMin = val;
        }
    }

    // --- 3️⃣ 范围更新 ---
    // 动态范围：取最大值往下推 60dB，这样效果最清晰
    double zMax = dataMax;
    double zMin = qMax(dataMin, zMax - 60.0);
    colorMap->setDataRange(QCPRange(zMin, zMax));

    // --- 4️⃣ 刷新视图 ---
    // 坐标轴只有在第一次或窗口变化时需要设置
    customPlot->rescaleAxes();
    customPlot->replot(QCustomPlot::rpQueuedReplot); // 异步重绘提高响应
}
void showRDMap(QCustomPlot *customPlot, const float* rdmap, int fftSize, int rngNum )
{
    if (!customPlot || !rdmap) return;

    // 清理旧图层
    // customPlot->clearPlottables();
    // customPlot->clearGraphs();

    updateRangeDopplerPlot(customPlot,rdmap);
}

void Save8T8VectorToBin(const std::string& filename,
                        const std::vector<pointandtimedata8x8>& data) {

    // 1. 打开文件
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // 2. 写入总帧数 (文件头)
    uint64_t frameCount = static_cast<uint64_t>(data.size());
    ofs.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));
    if (ofs.fail()) {
        throw std::runtime_error("Error writing frame count.");
    }

    // 3. 循环写入每一帧数据
    for (const auto& frame : data) {

        // --- 3.1 写入 POD 字段 ---
        uint64_t indexNo = static_cast<uint64_t>(frame.indexno);
        uint64_t startTimeNs = frame.start_time_ns;
        uint64_t timestampNs = frame.timestamp_ns;

        ofs.write(reinterpret_cast<const char*>(&indexNo), sizeof(indexNo));
        ofs.write(reinterpret_cast<const char*>(&startTimeNs), sizeof(startTimeNs));
        ofs.write(reinterpret_cast<const char*>(&timestampNs), sizeof(timestampNs));

        // --- 3.2 写入 RD Map (std::vector<float>) ---
        uint64_t rdmap_size = static_cast<uint64_t>(frame.Radar8T8Point.rdmap.size());

        // 写入 RD Map 数组大小
        ofs.write(reinterpret_cast<const char*>(&rdmap_size), sizeof(rdmap_size));

        if (rdmap_size > 0) {
            // 写入 RD Map 数组数据
            ofs.write(reinterpret_cast<const char*>(frame.Radar8T8Point.rdmap.data()),
                      rdmap_size * sizeof(float));
        }

        // --- 3.3 写入 PCL 点云 (shared_ptr<pcl::PointCloud<...>>) ---
        uint64_t point_count = 0;
        const auto& cloud_ptr = frame.Radar8T8Point.cloud;

        if (cloud_ptr && !cloud_ptr->empty()) {
            point_count = static_cast<uint64_t>(cloud_ptr->points.size());
        }

        // 写入点云数量
        ofs.write(reinterpret_cast<const char*>(&point_count), sizeof(point_count));

        if (point_count > 0) {
            // 写入点云数据
            ofs.write(reinterpret_cast<const char*>(cloud_ptr->points.data()),
                      point_count * sizeof(PointXYZRGBWithProperties));
        }

        if (ofs.fail()) {
            throw std::runtime_error("Error writing frame data at index " + std::to_string(frame.indexno));
        }
    }

    ofs.close();
}

// =================================================================
// 1. 初始化文件：写入占位符 (在主循环开始前调用)
// =================================================================
void Init8T8BinFile(const std::string& filename_qt) {

    std::wstring filename = utf8_to_wstring(filename_qt);
    // 强制打开文件并截断（清空），写入 8 字节的 0 作为帧数占位符。
    std::ofstream ofs(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for initialization: " );
    }

    uint64_t placeholder = 0;
    ofs.write(reinterpret_cast<const char*>(&placeholder), sizeof(placeholder));

    if (ofs.fail()) {
        throw std::runtime_error("Error writing frame count placeholder.");
    }
    // 文件关闭后，新的逐帧写入将使用 std::ios::app 从占位符之后开始。
}

// =================================================================
// 2. 逐帧写入 (使用您提供的逻辑)
// =================================================================
// 假设 pointandtimedata8x8 和 PointXYZRGBWithProperties 在其他头文件中定义

void Save8T8FrameToBin(const std::string& filename,
                       const pointandtimedata8x8& frame) {
    // 1. 打开文件，使用追加模式 (std::ios::app)
    // 注意：这将从 Init8T8BinFile 写入的占位符之后开始写入。
    std::ofstream ofs(filename, std::ios::binary | std::ios::app);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing/appending: " + filename);
    }

    // --- 写入 POD 字段 ---
    uint64_t indexNo = static_cast<uint64_t>(frame.indexno);
    uint64_t startTimeNs = frame.start_time_ns;
    uint64_t timestampNs = frame.timestamp_ns;

    ofs.write(reinterpret_cast<const char*>(&indexNo), sizeof(indexNo));
    ofs.write(reinterpret_cast<const char*>(&startTimeNs), sizeof(startTimeNs));
    ofs.write(reinterpret_cast<const char*>(&timestampNs), sizeof(timestampNs));

    // --- 写入 RD Map (std::vector<float>) ---
    uint64_t rdmap_size = static_cast<uint64_t>(frame.Radar8T8Point.rdmap.size());
    ofs.write(reinterpret_cast<const char*>(&rdmap_size), sizeof(rdmap_size));

    if (rdmap_size > 0) {
        ofs.write(reinterpret_cast<const char*>(frame.Radar8T8Point.rdmap.data()),
                  rdmap_size * sizeof(float));
    }

    // --- 写入 PCL 点云 (shared_ptr<pcl::PointCloud<...>>) ---
    uint64_t point_count = 0;
    const auto& cloud_ptr = frame.Radar8T8Point.cloud;

    if (cloud_ptr && !cloud_ptr->empty()) {
        point_count = static_cast<uint64_t>(cloud_ptr->points.size());
    }
    ofs.write(reinterpret_cast<const char*>(&point_count), sizeof(point_count));

    if (point_count > 0) {
        ofs.write(reinterpret_cast<const char*>(cloud_ptr->points.data()),
                  point_count * sizeof(PointXYZRGBWithProperties));
    }

    if (ofs.fail()) {
        throw std::runtime_error("Error writing frame data at index " + std::to_string(frame.indexno) + " during append.");
    }

    ofs.close();
}

// =================================================================
// 3. 终结文件：回填总帧数 (在主循环结束后调用)
// =================================================================
void Finalize8T8BinFile(const std::string& filename, uint64_t finalFrameCount) {
    // 打开文件进行写入，不使用追加模式，以便我们可以 seekp 到开头。
    // 使用 std::ios::in | std::ios::out 允许读写
    std::fstream fs(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!fs) {
        throw std::runtime_error("Failed to open file for finalization: " + filename);
    }

    // 将文件指针移动到开头 (位置 0)
    fs.seekp(0);

    // 写入实际的总帧数
    fs.write(reinterpret_cast<const char*>(&finalFrameCount), sizeof(finalFrameCount));

    if (fs.fail()) {
        qWarning() << "Error writing final frame count to file:" << QString::fromStdString(filename);
        // 不抛出异常，继续执行
    }

    fs.close();
}


bool LoadSingle8T8Frame(std::ifstream& ifs, pointandtimedata8x8& data)
{
    constexpr uint64_t MAX_RDMAP_SIZE   = 1024 * 1024;   // 1M float
    constexpr uint64_t MAX_POINT_COUNT  = 500000;       // 50 万点

    // --- 1. 读取 POD 字段 ---
    uint64_t indexNo, startTimeNs, timestampNs;
    if (!ifs.read(reinterpret_cast<char*>(&indexNo), sizeof(indexNo)) ||
        !ifs.read(reinterpret_cast<char*>(&startTimeNs), sizeof(startTimeNs)) ||
        !ifs.read(reinterpret_cast<char*>(&timestampNs), sizeof(timestampNs))) {
        return false;
    }

    data.indexno = static_cast<size_t>(indexNo);
    data.start_time_ns = startTimeNs;
    data.timestamp_ns = timestampNs;

    // --- 2. 读取 RD Map (std::vector<float>) ---
    uint64_t rdmap_size = 0;
    if (!ifs.read(reinterpret_cast<char*>(&rdmap_size), sizeof(rdmap_size)) ||
        rdmap_size > MAX_RDMAP_SIZE) {
        throw std::runtime_error("Invalid rdmap size");
    }

    data.Radar8T8Point.rdmap.resize(rdmap_size);
    if (rdmap_size > 0) {
        if (!ifs.read(reinterpret_cast<char*>(data.Radar8T8Point.rdmap.data()),
                      rdmap_size * sizeof(float))) {
            throw std::runtime_error("Failed to read rdmap data");
        }
    }

    // --- 3. 读取 PCL 点云 (shared_ptr<pcl::PointCloud<...>>) ---
    uint64_t point_count = 0;
    if (!ifs.read(reinterpret_cast<char*>(&point_count), sizeof(point_count)) ||
        point_count > MAX_POINT_COUNT) {
        throw std::runtime_error("Invalid point count");
    }


    if (point_count > 0) {
        auto cloud = std::make_shared<pcl::PointCloud<PointXYZRGBWithProperties>>();
        cloud->points.resize(point_count);

        if (!ifs.read(reinterpret_cast<char*>(cloud->points.data()),
                      point_count * sizeof(PointXYZRGBWithProperties))) {
            throw std::runtime_error("Failed to read point cloud data");
        }

        data.Radar8T8Point.cloud = cloud;
    } else {
        data.Radar8T8Point.cloud.reset();
    }

    return true;
}


std::wstring utf8_to_wstring(const std::string& str)
{
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

std::vector<pointandtimedata8x8> Load8T8VectorFromBin(const std::string& filename_qt) {

    std::wstring filename = utf8_to_wstring(filename_qt);

    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open file for reading ");
    }

    // 1. 读取总帧数
    uint64_t frameCount = 0;
    if (!ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(frameCount)) ||
        frameCount == 0 || frameCount > 50000) {
        throw std::runtime_error("Invalid 8x8 BIN: frame count");
    }



    std::vector<pointandtimedata8x8> result;
    result.reserve(frameCount);


    // 2. 循环读取每一帧数据
    for (uint64_t i = 0; i < frameCount; ++i) {
        pointandtimedata8x8 frame;

        try {
            LoadSingle8T8Frame(ifs, frame);  // 建议内部 throw
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "Invalid 8x8 BIN at frame " + std::to_string(i) + ": " + e.what()
                );
        }
        result.push_back(std::move(frame));
    }

    ifs.close();
    return result;
}
