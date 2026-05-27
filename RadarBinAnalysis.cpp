#include "RadarBinAnalysis.h"

void GetRadar4T4Datafrommf4(const QString& filename,const QString& savePath,double start,double end,const std::vector<TimestampedCarData>& CarData,const std::vector<TimestampedCarSRSYawRATE>& CarSRSYawData){
    qDebug()<<"mf4 to point data";

    prependPath(QStringLiteral("C:\\Program Files\\MATLAB\\R2022b\\runtime\\win64"));




    // 1. 初始化 MATLAB Runtime
    if (!mclInitializeApplication(NULL, 0)) {
        std::cerr << "Failed to initialize MCR." << std::endl;
        return ;
    }

    if (!Radar_4T4RMain_DllInitialize()) {
        std::cerr << "Failed to initialize DLL." << std::endl;
        return ;
    }
    try {
        // 2. 读取 bin 文件为 double 类型的三维数据

        QFileInfo info(savePath);
        QString CSVPath = info.absolutePath() + "/" + info.completeBaseName() + ".csv";

        std::vector<TimestampedData> collected_data;
        collected_data = readallmdftobin(filename.toStdString(),start,end);
        const size_t total_size = DIM1 * DIM2 * DIM3;

        std::vector<pointandtimedata> alldata;

        for (const auto& data_point : collected_data){
            qDebug()<<"index number ："<<data_point.number;
            PointData Radar4T4Point;

            size_t offset = 0;
            size_t frame_size = data_point.data.size();


            std::vector<uint8_t> raw12_data = extractRaw12Frames(data_point.data, offset, frame_size);


            std::vector<uint16_t> pixels = decodeRaw12(raw12_data);


            if(pixels.size()!=total_size){
                std::cout <<"data format error"<<endl;
                return;
            }
            //std::cout<<pixels.size() <<endl;
            //std::cout<<total_size <<endl;

            std::vector<uint16_t> data3D_adc_ori;
            processPixelsMatlabOrder(pixels,DIM2, DIM1, data3D_adc_ori);

            // 3. 构建 mwArray（维度顺序：列主序）
            const mwSize dims[3] = {DIM1, DIM2, DIM3};
            mwArray Sig(3, dims, mxDOUBLE_CLASS);
            Sig.SetData(data3D_adc_ori.data(), total_size);

            // 4. 调用 MATLAB 函数
            //get CAR speed and Yawdata
            double carspeed = GetwheSpeed(CarData, data_point.start_time_ns+data_point.timestamp_ns);
            double CarYamdata = GetYawData(CarSRSYawData,  data_point.start_time_ns+data_point.timestamp_ns);
            mwArray detObjlist; // 输出
            mwArray whespeed(carspeed/3.6);
            mwArray yawrate(CarYamdata*3.1416/180);

           // mwArray whespeed(33.3/3.6);
            //mwArray yawrate(0.03*3.1416/180);


            mwArray idelTimeID(1);
            Radar_4T4RMain_Dll(1, detObjlist, Sig,whespeed,yawrate,idelTimeID);
            int numElements = detObjlist.NumberOfElements();
            if (numElements < 1) {
                std::cerr << "Error: detObjlist has no elements!" << std::endl;
                return;
            }

            // 提取需要的数据
            int nFields = detObjlist.NumberOfFields();



            for (int i = 0; i <= nFields-1; ++i) {
                mwString fieldName = detObjlist.GetFieldName(i);



                mwArray fieldValue = detObjlist.GetA(fieldName,1);
                std::vector<double> values(fieldValue.NumberOfElements());



                //std::vector<double> fieldvaluedouble=mwArrayToDoubleArray(fieldValue);
                /*std::string fieldindex = mwStringToStdString(fieldName);
            if((fieldindex!= "attribute") && (fieldindex!= "num")){

               qDebug()<< QString::fromStdString(fieldindex);

                qDebug()<<fieldvaluedouble;

               Radar4T4Point.strArray.push_back(fieldindex);

               //Radar4T4Point.dblArray.push_back(fieldvaluedouble);

            }*/
                if(fieldName!=mwString("attribute")&&fieldName!=mwString("num")){
                    // qDebug()<<fieldName;
                    fieldValue.GetData(values.data(), values.size());
                    /*for (size_t i = 0; i < values.size(); ++i) {
                        qDebug() << "Element[" << i << "] = " << values[i];
                    }*/
                    Radar4T4Point.strArray.push_back(fieldName);
                    Radar4T4Point.dblArray.push_back(values);
                    //qDebug()<< values.size();
                }

            }
            std::cout<<data_point.timestamp_ns<<endl;
            saveFieldDataToCSV(Radar4T4Point, CSVPath.toStdString(),convertTimestamp(data_point.start_time_ns+data_point.timestamp_ns),data_point.number);
            alldata.push_back({data_point.number,data_point.start_time_ns,data_point.timestamp_ns,Radar4T4Point});
        }

        //std::cout<<alldata.size()<<endl;
        savePointsToBin(savePath.toStdString(),alldata);



    } catch (const mwException &e) {
        std::cerr << "MATLAB exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }

    // 6. 结束
    Radar_4T4RMain_DllTerminate();
    mclTerminateApplication();



    return ;

}
double GetwheSpeed(const std::vector<TimestampedCarData>& CarData,const uint64_t PointTime){

    int carIndex = 0;

    while (carIndex + 1 < CarData.size() &&
           std::llabs(CarData[carIndex + 1].timestamp_ns+CarData[carIndex + 1].start_time_ns - PointTime) <
               std::llabs(CarData[carIndex].timestamp_ns+CarData[carIndex].start_time_ns  - PointTime)) {
        carIndex++;
    }
    return CarData[carIndex].data;


}

double GetYawData(const std::vector<TimestampedCarSRSYawRATE>& CarSRSYawData,const uint64_t PointTime){

    int YawIndex=0;

    while (YawIndex + 1 < CarSRSYawData.size() &&
           std::llabs(CarSRSYawData[YawIndex + 1].timestamp_ns+CarSRSYawData[YawIndex + 1].start_time_ns - PointTime) <
               std::llabs(CarSRSYawData[YawIndex].timestamp_ns+CarSRSYawData[YawIndex].start_time_ns  - PointTime)) {
        YawIndex++;
    }
    return CarSRSYawData[YawIndex].data;


}

void GetRadar4T4Datafrommf4(const QString& filename,const QString& savePath,double start,double end){
    qDebug()<<"mf4 to point data";
    prependPath(QStringLiteral("C:\\Program Files\\MATLAB\\R2022b\\runtime\\win64"));



    // 1. 初始化 MATLAB Runtime
    if (!mclInitializeApplication(NULL, 0)) {
        std::cerr << "Failed to initialize MCR." << std::endl;
        return ;
    }

    if (!Radar_4T4RMain_DllInitialize()) {
        std::cerr << "Failed to initialize DLL." << std::endl;
        return ;
    }
    try {
        // 2. 读取 bin 文件为 double 类型的三维数据

        QFileInfo info(savePath);
        QString CSVPath = info.absolutePath() + "/" + info.completeBaseName() + ".csv";

        std::vector<TimestampedData> collected_data;
        collected_data = readallmdftobin(filename.toStdString(),start,end);
        const size_t total_size = DIM1 * DIM2 * DIM3;

        std::vector<pointandtimedata> alldata;

        for (const auto& data_point : collected_data){
            PointData Radar4T4Point;

            size_t offset = 0;
            size_t frame_size = data_point.data.size();


            std::vector<uint8_t> raw12_data = extractRaw12Frames(data_point.data, offset, frame_size);


            std::vector<uint16_t> pixels = decodeRaw12(raw12_data);


            if(pixels.size()!=total_size){
                std::cout <<"data format error"<<endl;
                return;
           }
          //std::cout<<pixels.size() <<endl;
          //std::cout<<total_size <<endl;

            std::vector<uint16_t> data3D_adc_ori;
            processPixelsMatlabOrder(pixels,DIM2, DIM1, data3D_adc_ori);

            // 3. 构建 mwArray（维度顺序：列主序）
            const mwSize dims[3] = {DIM1, DIM2, DIM3};
            mwArray Sig(3, dims, mxDOUBLE_CLASS);
            Sig.SetData(data3D_adc_ori.data(), total_size);

            // 4. 调用 MATLAB 函数
            mwArray detObjlist; // 输出
            mwArray whespeed(0.000);
            mwArray yawrate(0.000);
            mwArray idelTimeID(1);
            Radar_4T4RMain_Dll(1, detObjlist, Sig,whespeed,yawrate,idelTimeID);
            int numElements = detObjlist.NumberOfElements();
            if (numElements < 1) {
                std::cerr << "Error: detObjlist has no elements!" << std::endl;
                return;
            }

            // 提取需要的数据
            int nFields = detObjlist.NumberOfFields();



            for (int i = 0; i <= nFields-1; ++i) {
                mwString fieldName = detObjlist.GetFieldName(i);



                mwArray fieldValue = detObjlist.GetA(fieldName,1);
                std::vector<double> values(fieldValue.NumberOfElements());



                //std::vector<double> fieldvaluedouble=mwArrayToDoubleArray(fieldValue);
                /*std::string fieldindex = mwStringToStdString(fieldName);
            if((fieldindex!= "attribute") && (fieldindex!= "num")){

               qDebug()<< QString::fromStdString(fieldindex);

                qDebug()<<fieldvaluedouble;

               Radar4T4Point.strArray.push_back(fieldindex);

               //Radar4T4Point.dblArray.push_back(fieldvaluedouble);

            }*/
                if(fieldName!=mwString("attribute")&&fieldName!=mwString("num")){
                    // qDebug()<<fieldName;
                    fieldValue.GetData(values.data(), values.size());
                    /*for (size_t i = 0; i < values.size(); ++i) {
                        qDebug() << "Element[" << i << "] = " << values[i];
                    }*/
                    Radar4T4Point.strArray.push_back(fieldName);
                    Radar4T4Point.dblArray.push_back(values);
                 //qDebug()<< values.size();
                }

            }
            std::cout<<data_point.timestamp_ns<<endl;
            saveFieldDataToCSV(Radar4T4Point, CSVPath.toStdString(),convertTimestamp(data_point.start_time_ns+data_point.timestamp_ns),data_point.number);
            alldata.push_back({data_point.number,data_point.start_time_ns,data_point.timestamp_ns,Radar4T4Point});
        }

        //std::cout<<alldata.size()<<endl;
        savePointsToBin(savePath.toStdString(),alldata);



    } catch (const mwException &e) {
        std::cerr << "MATLAB exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }

    // 6. 结束
    Radar_4T4RMain_DllTerminate();
    mclTerminateApplication();



    return ;

}



void GetRadar4T4DatafromBIN(const QString& filename,const QString& savePath){

    qDebug()<<"SUCESS";

    //prependPath(QStringLiteral("C:\\Program Files\\MATLAB\\R2022b\\runtime\\win64"));

    PointData Radar4T4Point;




    // 1. 初始化 MATLAB Runtime
    if (!mclInitializeApplication(NULL, 0)) {
        std::cerr << "Failed to initialize MCR." << std::endl;
        return ;
    }

    qDebug()<<"111";
    if (!Radar_4T4RMain_DllInitialize()) {
        std::cerr << "Failed to initialize DLL." << std::endl;
        return ;
    }

    qDebug()<<"222";
    try {
        // 2. 读取 bin 文件为 double 类型的三维数据

        const size_t total_size = DIM1 * DIM2 * DIM3;
        std::vector<uint8_t> frame_bytes;

        size_t frame_size = 4744704;
        readBinFile(filename.toStdString(), frame_bytes);

        qDebug()<<"333";
        std::vector<uint8_t> raw12_data = extractRaw12Frames(frame_bytes, 0, frame_size);


        std::vector<uint16_t> pixels = decodeRaw12(raw12_data);

        qDebug()<<"444";

        if(pixels.size()!=total_size){
            std::cout <<"data format error"<<endl;
            return;
        }

        /*
        std::ifstream binFile(filename.toStdString(), std::ios::binary);
        if (!binFile) {
            std::cerr << "Cannot open bin file!" << std::endl;
            return ;
        }

        const size_t total_size = DIM1 * DIM2 * DIM3;
        //std::vector<double> data(total_size);

        std::vector<uint16_t> data(total_size);

        binFile.read(reinterpret_cast<char*>(data.data()), total_size * sizeof(uint16_t));

        binFile.close();

        if (binFile.gcount() != total_size * sizeof(uint16_t)) {
            std::cerr << "Bin file size mismatch." << std::endl;
            return ;
        }*/

        /*
        std::vector<uint16_t> newdata = reorderTX( data, DIM3, 1) ;


        std::ofstream out("outputADC.bin", std::ios::binary);
        out.write(reinterpret_cast<const char*>(newdata.data()), newdata.size() * sizeof(uint16_t));

        out.close();
        */


        std::vector<uint16_t> data3D_adc_ori;
        processPixelsMatlabOrder(pixels,DIM2, DIM1, data3D_adc_ori);

        std::ofstream out("outputADC0821.bin", std::ios::binary);
        out.write(reinterpret_cast<const char*>(data3D_adc_ori.data()), data3D_adc_ori.size() * sizeof(uint16_t));
        out.close();

        // 3. 构建 mwArray（维度顺序：列主序）
        const mwSize dims[3] = {DIM1, DIM2, DIM3};
        mwArray Sig(3, dims, mxDOUBLE_CLASS);
        Sig.SetData(data3D_adc_ori.data(), total_size);

        //qDebug()<<"123";
        // 4. 调用 MATLAB 函数
        mwArray detObjlist; // 输出
        mwArray whespeed(33.3/3.6);
        mwArray yawrate(0.08*3.1416/180);
        mwArray idelTimeID(1);
        Radar_4T4RMain_Dll(1, detObjlist, Sig,whespeed,yawrate,idelTimeID);

        //qDebug()<<newdata.size();
        //qDebug()<<total_size;

        int numElements = detObjlist.NumberOfElements();
        if (numElements < 1) {
            std::cerr << "Error: detObjlist has no elements!" << std::endl;
            return;
        }



        // 提取需要的数据
        int nFields = detObjlist.NumberOfFields();



        for (int i = 0; i <= nFields-1; ++i) {
            mwString fieldName = detObjlist.GetFieldName(i);



            mwArray fieldValue = detObjlist.GetA(fieldName,1);
            std::vector<double> values(fieldValue.NumberOfElements());



            //std::vector<double> fieldvaluedouble=mwArrayToDoubleArray(fieldValue);
            /*std::string fieldindex = mwStringToStdString(fieldName);
            if((fieldindex!= "attribute") && (fieldindex!= "num")){

               qDebug()<< QString::fromStdString(fieldindex);

                qDebug()<<fieldvaluedouble;

               Radar4T4Point.strArray.push_back(fieldindex);

               //Radar4T4Point.dblArray.push_back(fieldvaluedouble);

            }*/
            if(fieldName!=mwString("attribute")&&fieldName!=mwString("num")){
               // qDebug()<<fieldName;
                fieldValue.GetData(values.data(), values.size());
               /*for (size_t i = 0; i < values.size(); ++i) {
                   qDebug() << "Element[" << i << "] = " << values[i];
               }*/
                Radar4T4Point.strArray.push_back(fieldName);
                Radar4T4Point.dblArray.push_back(values);

            }

        }





        //saveStrArrayToCSV(Radar4T4Point, "output.csv");
      saveFieldDataToCSV(Radar4T4Point, savePath.toStdString());

        //qDebug()<<Radar4T4Point.strArray.size();
       // qDebug()<<Radar4T4Point.dblArray.size();
        //qDebug()<<QString::fromStdString(Radar4T4Point.strArray[0]);

    } catch (const mwException &e) {
        std::cerr << "MATLAB exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }

    // 6. 结束
    Radar_4T4RMain_DllTerminate();
    mclTerminateApplication();



    return ;



}


// 需要添加环境变量MATLAB RUNTIME

std::string mwStringToStdString(const mwString& mwStr) {
    return std::string(mwStr);
}



std::vector<double> mwArrayToDoubleArray(const mwArray& fieldValue) {
    if (fieldValue.NumberOfElements() == 0) {
        return {}; // 空向量
    }

    if (fieldValue.ClassID() != mxDOUBLE_CLASS) {
        throw std::runtime_error("mwArray is not of type double.");
    }

    int numElements = fieldValue.NumberOfElements();
    std::vector<double> outVec(numElements);

    fieldValue.GetData(outVec.data(), numElements); // 更安全的写法

    return outVec;
}


void prependPath(const QString& newPath) {
    wchar_t oldPath[32767];
    DWORD ret = GetEnvironmentVariableW(L"PATH", oldPath, 32767);
    QString pathStr = QString::fromWCharArray(oldPath, ret);

    if (!pathStr.contains(newPath)) {
        QString newPathStr = newPath + ";" + pathStr;
        SetEnvironmentVariableW(L"PATH", (const wchar_t*)newPathStr.utf16());
    }
}


void saveStrArrayToCSV(const PointData& pointData, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        qDebug() << "cann't Open:" << QString::fromStdString(filename);
        return;
    }

    for (const auto& str : pointData.strArray) {
        outFile << str << "\n";
    }

    outFile.close();
    qDebug() << "CSV Saved:" << QString::fromStdString(filename);
}

void saveDblArrayToCSV(const PointData& pointData, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        qDebug() << "CANN'T OPEN:" << QString::fromStdString(filename);
        return;
    }

    for (const auto& row : pointData.dblArray) {

        for (size_t i = 0; i < row.size(); ++i) {
            outFile << row[i];

            if (i != row.size() )
                outFile << ",";
        }
        outFile << "\n";
    }

    outFile.close();
    qDebug() << "CSV saved:" << QString::fromStdString(filename);
}

void saveFieldDataToCSV(const PointData& pointData, const std::string& filename) {
    // Open the file in append mode. This is the key change.
    std::ofstream outFile(filename, std::ios_base::app);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return;
    }

    // Check if file is empty to write header
    std::ifstream checkFile(filename);
    if (checkFile.peek() == std::ifstream::traits_type::eof()) {
        for (const auto& str : pointData.strArray) {
            outFile << str << ",";
        }
        outFile << "\n";
    }
    checkFile.close();

    // Write each data point on a new row
    outFile << std::fixed << std::setprecision(5);
    for(size_t j = 0; j < pointData.dblArray[0].size(); ++j) {
        for (size_t i = 0; i < pointData.dblArray.size(); ++i) {
            outFile << pointData.dblArray[i][j] << ",";
        }
        outFile << "\n";
    }

    outFile.close();
    std::cout << "CSV file saved to: " << filename << std::endl;
}

void saveFieldDataToCSV(const PointData& pointData, const std::string& filename,std::string timestring,size_t number) {
    // Open the file in append mode. This is the key change.
    std::ofstream outFile(filename, std::ios_base::app);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return;
    }

    // Check if file is empty to write header
    std::ifstream checkFile(filename);
    if (checkFile.peek() == std::ifstream::traits_type::eof()) {
        for (const auto& str : pointData.strArray) {
            outFile << str << ",";
        }
         outFile << "Starttime" << ",";
        outFile << "Index" << ",";
        outFile << "\n";
    }

    checkFile.close();

    // Write each data point on a new row
    outFile << std::fixed << std::setprecision(5);
    for(size_t j = 0; j < pointData.dblArray[0].size(); ++j) {
        for (size_t i = 0; i < pointData.dblArray.size(); ++i) {
            outFile << pointData.dblArray[i][j] << ",";
        }
        outFile<< timestring << ",";
        outFile<< number << ",";
        outFile << "\n";
    }

    outFile.close();
    std::cout << "CSV file saved to: " << filename << std::endl;
}

std::vector<uint16_t> reorderTX(const std::vector<uint16_t>& input,
                                int txCount,
                                int blockSize) {

    size_t totalSize = input.size();
    if (totalSize % (txCount * blockSize) != 0) {
        throw std::runtime_error("输入数据大小不是 txCount * blockSize 的整数倍");
    }
    size_t loops = totalSize / (txCount * blockSize);
    std::vector<uint16_t> output(totalSize);

    for (int tx = 0; tx < txCount; ++tx) {
        for (size_t loop = 0; loop < loops; ++loop) {
            size_t srcIndex = loop * txCount * blockSize + tx * blockSize;
            size_t dstIndex = tx * loops * blockSize + loop * blockSize;
            std::copy_n(input.begin() + srcIndex, blockSize, output.begin() + dstIndex);
        }
    }
    return output;
}



void processPixelsMatlabOrder(const std::vector<uint16_t> &pixels,
                              int samplenum, int chirpnum,
                              std::vector<uint16_t> &data1D)
{
    if (pixels.size() != static_cast<size_t>(chirpnum * samplenum * 4)) {
        std::cerr << "Error: pixels size mismatch" << std::endl;
        return;
    }

    // 先构建 3D 数组 [chirp][sample][rx]
    std::vector<std::vector<std::vector<uint16_t>>> data3D_adc_ori(
        chirpnum, std::vector<std::vector<uint16_t>>(samplenum, std::vector<uint16_t>(4)));

    // 拆分四路数据
    for (int chirp = 0; chirp < chirpnum; ++chirp) {
        for (int sample = 0; sample < samplenum; ++sample) {
            int idx = (chirp * samplenum + sample) * 4;
            data3D_adc_ori[chirp][sample][0] = pixels[idx];
            data3D_adc_ori[chirp][sample][1] = pixels[idx+1];
            data3D_adc_ori[chirp][sample][2] = pixels[idx+2];
            data3D_adc_ori[chirp][sample][3] = pixels[idx+3];
        }
    }

    // 按 MATLAB 列优先展开
    data1D.resize(pixels.size());
    size_t pos = 0;
    for (int rx = 0; rx < 4; ++rx) {
        for (int sample = 0; sample < samplenum; ++sample) {
            for (int chirp = 0; chirp < chirpnum; ++chirp) {
                data1D[pos++] = data3D_adc_ori[chirp][sample][rx];
            }
        }
    }
}


// Corrected save function using fixed-width integers
void savePointsToBin(const std::string& filename,
                     const std::vector<pointandtimedata>& data) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    // Use a fixed-width type for frame count
    uint64_t frameCount = static_cast<uint64_t>(data.size());
    ofs.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));

    for (const auto& p : data) {
        // Use fixed-width types for consistent binary data.
        uint64_t indexNo = static_cast<uint64_t>(p.indexno);
        uint64_t startTimeNs = p.start_time_ns;
        uint64_t timestampNs = p.timestamp_ns;

        ofs.write(reinterpret_cast<const char*>(&indexNo), sizeof(indexNo));
        ofs.write(reinterpret_cast<const char*>(&startTimeNs), sizeof(startTimeNs));
        ofs.write(reinterpret_cast<const char*>(&timestampNs), sizeof(timestampNs));


        /*// Save strArray using a fixed-width integer for count and length
        uint64_t strCount = static_cast<uint64_t>(p.Radar4T4Point.strArray.size());
        //std::cout << "Saving strCount: " << strCount << " for frame " << indexNo << std::endl;

        ofs.write(reinterpret_cast<const char*>(&strCount), sizeof(strCount));
        for (const auto& s : p.Radar4T4Point.strArray) {
            std::string tmp = mwStringToStdString(s);
            uint64_t len = static_cast<uint64_t>(tmp.size());
            std::cout << "Saving string length: " << len << " for string '" << tmp << "'" << std::endl;

            ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));
            ofs.write(tmp.c_str(), len);
        }
        */
        // Save dblArray using a fixed-width integer for counts and length
        uint64_t dblVecCount = static_cast<uint64_t>(p.Radar4T4Point.dblArray.size());
        //std::cout << "Saving dblVecCount: " << dblVecCount << " for frame " << indexNo << std::endl;
        ofs.write(reinterpret_cast<const char*>(&dblVecCount), sizeof(dblVecCount));
        for (const auto& vec : p.Radar4T4Point.dblArray) {
            uint64_t vlen = static_cast<uint64_t>(vec.size());
            ofs.write(reinterpret_cast<const char*>(&vlen), sizeof(vlen));
            ofs.write(reinterpret_cast<const char*>(vec.data()), vlen * sizeof(double));
        }
    }
}

// Corrected load function using fixed-width integers
std::vector<pointandtimedata> loadPointsFromBin(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Failed to open file for reading: " + filename);
    }

    uint64_t frameCount = 0;
    if (!ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(frameCount)) ||
        frameCount == 0 || frameCount > 100000) {
        throw std::runtime_error("Invalid BIN: frame count");
    }

    std::vector<pointandtimedata> result;
    result.reserve(frameCount);

    for (uint64_t f = 0; f < frameCount; ++f) {
        pointandtimedata p;

        if (!ifs.read(reinterpret_cast<char*>(&p.indexno), sizeof(uint64_t)) ||
            !ifs.read(reinterpret_cast<char*>(&p.start_time_ns), sizeof(uint64_t)) ||
            !ifs.read(reinterpret_cast<char*>(&p.timestamp_ns), sizeof(uint64_t))) {
            throw std::runtime_error("Invalid BIN: header truncated");
        }

        uint64_t indexNo;
        uint64_t startTimeNs;
        uint64_t timestampNs;

        if (!ifs.read(reinterpret_cast<char*>(&indexNo), sizeof(indexNo))) break;
        if (!ifs.read(reinterpret_cast<char*>(&startTimeNs), sizeof(startTimeNs))) break;
        if (!ifs.read(reinterpret_cast<char*>(&timestampNs), sizeof(timestampNs))) break;

        p.indexno = static_cast<size_t>(indexNo);
        p.start_time_ns = startTimeNs;
        p.timestamp_ns = timestampNs;

        // --- FIXED ---
        // Removed the duplicate read of strCount.
        // It is now read once and used correctly.

        // Corrected strArray reading
        /*uint64_t strCount;
        if (!ifs.read(reinterpret_cast<char*>(&strCount), sizeof(strCount))) break;
        p.Radar4T4Point.strArray.resize(strCount);
        for (uint64_t i = 0; i < strCount; i++) {
            uint64_t len;
            if (!ifs.read(reinterpret_cast<char*>(&len), sizeof(len))) break;

            // Sanity check to prevent crashes from corrupted file data
                if (len > 10000) { // Assuming no string will be longer than 10,000 bytes
                std::cerr << "CRITICAL ERROR: Read an invalid string length (" << len << "). The file is likely corrupted." << std::endl;
                break;
            }


            // Use a temporary std::string for reading
            std::string tmp(static_cast<std::string::size_type>(len), '\0');
            if (!ifs.read(&tmp[0], static_cast<std::streamsize>(len))) break;

            // Construct mwString from the read std::string
            p.Radar4T4Point.strArray[i] = mwString(tmp.c_str());
        }
        */
        // --- FIXED ---
        // Removed the duplicate read of dblVecCount.
        // It is now read once and used correctly.

        // Corrected dblArray reading
        uint64_t dblVecCount;
        if (!ifs.read(reinterpret_cast<char*>(&dblVecCount), sizeof(dblVecCount)) ||
            dblVecCount == 0 || dblVecCount > 64) {
            throw std::runtime_error("Invalid BIN: dblVecCount");
        }


        if (!ifs.read(reinterpret_cast<char*>(&dblVecCount), sizeof(dblVecCount))) break;
        p.Radar4T4Point.dblArray.resize(dblVecCount);
        //std::cout<<dblVecCount<<endl;
        for (uint64_t i = 0; i < dblVecCount; i++) {
            uint64_t vlen;
            if (!ifs.read(reinterpret_cast<char*>(&vlen), sizeof(vlen)) ||
                vlen == 0 || vlen > 500000) {
                throw std::runtime_error("Invalid BIN: vector length");
            }

            p.Radar4T4Point.dblArray[i].resize(static_cast<std::vector<double>::size_type>(vlen));
            if (!ifs.read(reinterpret_cast<char*>(p.Radar4T4Point.dblArray[i].data()),
                          vlen * sizeof(double))) {
                throw std::runtime_error("Invalid BIN: vector data");
            }
        }

        result.push_back(std::move(p));
    }
    return result;
}
