
#include "mdf2bin.h"
#include "rspprocess.h"
#include <QFile>
#include <QDir>


std::string convertTimestamp(uint64_t nanoseconds) {
    // 1. 从纳秒值创建一个 duration 对象。
    auto ns_duration = std::chrono::nanoseconds(nanoseconds);

    // 2. 从 duration 创建一个具有纳秒精度的 time_point。
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time_point_ns(ns_duration);

    // 3. 将 time_point 显式转换为 system_clock 的本地精度。
    // 这通常是纳秒或更粗的精度，但为了与 std::time_t 兼容，我们进行此转换。
    auto time_point_native = std::chrono::time_point_cast<std::chrono::system_clock::duration>(time_point_ns);

    // 4. 将本地 time_point 转换为 std::time_t。
    // 注意：这将截断任何亚秒部分，因为 std::time_t 只有秒级精度。
    std::time_t time_t_value = std::chrono::system_clock::to_time_t(time_point_native);

    // 5. 将 time_t 转换为本地时间结构体。
    std::tm* local_tm = std::localtime(&time_t_value);

    // 6. 使用 strftime 格式化日期和时间，只保留到秒。
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_tm);

    // 7. 重新计算亚秒部分，并将其转换为纳秒。
    // 这一步是关键，它确保我们保留了完整的纳秒精度。
    auto truncated_to_seconds = std::chrono::time_point_cast<std::chrono::seconds>(time_point_ns);
    auto remaining_duration = time_point_ns - truncated_to_seconds;
    auto milliseconds_part = std::chrono::duration_cast<std::chrono::milliseconds>(remaining_duration).count();
    // 8. 使用 stringstream 来构建最终的格式化字符串，并用9位宽度和0填充。
    std::stringstream ss;
    ss << buffer << "." << std::setw(3) << std::setfill('0') << milliseconds_part;

    // 9. 返回构建好的字符串。
    return ss.str();
}

std::vector<int16_t> process_matlab_typecast_and_bitshift(
    const std::vector<uint16_t>& pixels,
    int shift_amount)
{
    // 预分配内存以提高效率
    std::vector<int16_t> result_flat;
    result_flat.reserve(pixels.size());

    for (uint16_t u16_val : pixels) {
        // --- 3. typecast(uint16_t, 'int16') - 位模式重新解释 ---
        // 使用 memcpy 是最安全和符合标准的方式来实现位模式重新解释
        int16_t i16_val;
        std::memcpy(&i16_val, &u16_val, sizeof(uint16_t));

        // --- 4. bitshift(..., 4) - 左移指定的位数 ---
        // 对 int16_t 进行左移操作
        i16_val = i16_val << shift_amount;

        result_flat.push_back(i16_val);
    }

    return result_flat;
}

std::vector<uint16_t> TransferOneFrame8_8RadarADC(const QString& filename){

    std::vector<uint8_t> frame_bytes;
    // std::string filename = "C:/Users/JPN1WX/Desktop/QT/PointCloudUI/build/Desktop_Qt_5_15_2_MSVC2019_64bit-Release/8x8data/Recorder_2025-11-12_16-42-25_Str_Radar_demo2_10.bin";
    readBinFile(filename.toStdString(), frame_bytes);

    int width = 1024;
    int height =512;
    int chirpnum=8;
    size_t frame_size = (width*chirpnum/2*3+58)*height;

    size_t total_frames = frame_bytes.size() / frame_size;
    size_t current_frame = 0;
    size_t offset = current_frame * frame_size;
    std::vector<uint8_t> raw12_data = extractRaw12Frames8x8(frame_bytes, offset, frame_size);
    std::vector<uint16_t> pixels = decodeRaw12(raw12_data);

    // std::ofstream out("output8x8.bin", std::ios::binary);
    // out.write(reinterpret_cast<const char*>(pixels.data()), pixels.size() * sizeof(uint16_t));

    // out.close();


    return pixels;
}


void TransferOneFrame2RadarADC(){

    //readallmdftobin();
    std::vector<uint8_t> frame_bytes;
    std::string filename = "output.bin";


    readBinFile(filename, frame_bytes);
    //qDebug()<<frame_bytes.size();


    int width = 4096;
    int height =768;
    size_t frame_size = 4744704;
    size_t total_frames = frame_bytes.size() / frame_size;
    size_t current_frame = 0;


    size_t offset = current_frame * frame_size;
    std::vector<uint8_t> raw12_data = extractRaw12Frames(frame_bytes, offset, frame_size);
    std::vector<uint16_t> pixels = decodeRaw12(raw12_data);

    std::ofstream out("outputONE.bin", std::ios::binary);
    out.write(reinterpret_cast<const char*>(pixels.data()), pixels.size() * sizeof(uint16_t));

    out.close();

}

bool readBinFile(const std::string& filenameqt, std::vector<uint8_t>& frame_bytes) {

    std::wstring filename=utf8_to_wstring(filenameqt);

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "cannt open: " << filenameqt << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();  // 文件大小
    file.seekg(0, std::ios::beg);

    frame_bytes.resize(size);
    if (!file.read(reinterpret_cast<char*>(frame_bytes.data()), size)) {
        std::cerr << "read file: " << filenameqt << std::endl;
        return false;
    }

    return true;
}


std::vector<TimestampedData> readallmdftobin(std::string filename){

    std::vector<TimestampedData> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();



    const auto* mdf_file = Reader.GetFile();

    const auto start_time_ns=mdf_file->Header()->StartTime();


    mdf::DataGroupList dg_list;

    mdf_file->DataGroups(dg_list);


    for (auto* dg4 : dg_list) {
        // Subscribers holds the sample data for a channel.
        // You should normally only subscribe on some channels.
        // We need a list to hold them.

        mdf::ChannelObserverList subscriber_list;
        const auto cg_list = dg4->ChannelGroups();
        for (const auto* cg4 : cg_list ) {

            const auto cn_list = cg4->Channels();
            for (const auto* cn4 : cn_list) {


                // Create a subscriber and add it to the temporary list
                auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
                subscriber_list.emplace_back(std::move(sub));
            }
        }


        // Now it is time to read in all samples
        Reader.ReadData(*dg4); // Read raw data from file








        std::vector<uint8_t> allbytes;

        allbytes.clear();

        std::vector<uint64_t> time_mapping;

        for (auto& obs : subscriber_list) {
            if (obs->Name() == "t") {
                size_t n = obs->NofSamples();
                time_mapping.resize(n);
                for (size_t sample = 0; sample < n; ++sample) {
                    auto relative_time_str = obs->EngValueToString(sample);
                    double relative_time_sec = std::stod(relative_time_str);
                    time_mapping[sample] = static_cast<uint64_t>(relative_time_sec * 1e9);
                }
            }
        }

        //std::ofstream outFile("output.bin", std::ios::binary);
        for (auto& obs : subscriber_list) {
            if (obs->Name() == "VideoRawdata_VC0") {
                size_t n = obs->NofSamples();
                for (size_t sample = 0; sample < n; ++sample) {
                    std::vector<uint8_t> bytes;
                    obs->GetChannelValue(sample, bytes);
                    if (!bytes.empty() && sample < time_mapping.size()) {

                        collected_data.push_back({start_time_ns, time_mapping[sample], bytes,sample});
                    }
                }
            }
        }
        //outFile.write(reinterpret_cast<const char*>(allbytes.data()), allbytes.size());
        //outFile.close();

        //qDebug() << "Channel Value (hex):" << allbytes.size();

        // Not needed in this example as we delete the subscribers,
        // but it is good practise to remove samples data from memory
        // when it is no longer needed.
        dg4->ClearData();
    }
    Reader.Close(); // Close the file

    return collected_data;

}

std::pair<size_t, size_t> TimeToFrameRange(double start, double end, size_t total_frames) {
    constexpr double frame_duration = 0.05; // 每帧 50ms
    constexpr size_t margin = 1;            // 向前/向后多取的帧数

    // 特殊情况：start=end=0.0 表示取全部
    if (start == 0.0 && end == 0.0) {
        return {0, total_frames > 0 ? total_frames  : 0};
    }

    // 时间转帧
    size_t start_frame = static_cast<size_t>(std::floor(start / frame_duration));
    size_t end_frame   = static_cast<size_t>(std::ceil(end  / frame_duration));

    // 加 margin 放宽
    if (start_frame >= margin) {
        start_frame -= margin;
    } else {
        start_frame = 0;
    }
    end_frame += margin;

    // 边界修正
    if (start_frame >= total_frames) {
        start_frame = total_frames > 0 ? total_frames - 1 : 0;
    }
    if (end_frame >= total_frames) {
        end_frame = total_frames > 0 ? total_frames - 1 : 0;
    }

    // 确保 start_frame <= end_frame
    if (start_frame > end_frame) {
        start_frame = end_frame;
    }

    return {start_frame, end_frame};
}

size_t readallmdftobin_by_frame(const std::string& filename,
                                const std::string& output_dir,
                                size_t req_start_frame,
                                size_t req_end_frame,
                                const std::string& video_channel_name ) {

    mdf::MdfReader Reader(filename);
    if (!Reader.IsOk()) {
        std::cerr << "Error: Could not open MDF file: " << filename << std::endl;
        return 0;
    }

    Reader.ReadEverythingButData();

    const auto* mdf_file = Reader.GetFile();
    // const auto start_time_ns = mdf_file->Header()->StartTime(); // 文件绝对起始时间，可以用于文件名

    // 检查输出目录是否存在，如果不存在尝试创建
    fs::path out_dir_path(output_dir);
    if (!fs::exists(out_dir_path)) {
        std::cout << "Output directory does not exist. Attempting to create: " << output_dir << std::endl;
        if (!fs::create_directories(out_dir_path)) {
            std::cerr << "Error: Failed to create output directory: " << output_dir << std::endl;
            return 0;
        }
    }

    size_t total_saved_files = 0;
    mdf::DataGroupList dg_list;
    mdf_file->DataGroups(dg_list);

    for (auto* dg4 : dg_list) {
        for (auto* cg4 : dg4->ChannelGroups()) {
            const auto* cn_t = cg4->GetChannel("t");
            const auto* cn_video = cg4->GetChannel(video_channel_name);

            if (!cn_t || !cn_video) {
                // std::cerr << "Warning: Channel 't' or '" << video_channel_name << "' not found in ChannelGroup '" << cg4->Name() << "'. Skipping." << std::endl;
                continue;
            }

            auto v_obs_for_total = CreateChannelObserver(*dg4, *cg4, *cn_video);
            size_t channel_total_frames = v_obs_for_total->NofSamples();

            size_t start_frame = req_start_frame;
            if (start_frame >= channel_total_frames) {
                // std::cout << "Warning: Requested start frame " << start_frame << " beyond total " << channel_total_frames << ". Skipping group." << std::endl;
                continue;
            }
            size_t end_frame = req_end_frame;
            if (end_frame > channel_total_frames) {
                end_frame = channel_total_frames;
            }
            if (start_frame >= end_frame) {
                // std::cout << "Warning: Invalid frame range (start >= end) " << start_frame << " to " << end_frame << ". Skipping group." << std::endl;
                continue;
            }
            std::cout << "Processing video frames from index " << start_frame << " to " << (end_frame - 1) << "..." << std::endl;
            // --------------------------------------------------------------------------------------


            // ⚡ 第一步：读取时间通道数据
            // 保持与原始函数一致，直接用全局索引循环
            Reader.ReadPartialData(*dg4, start_frame, end_frame);
            auto t_obs = CreateChannelObserver(*dg4, *cg4, *cn_t);

            std::vector<uint64_t> time_mapping;
            time_mapping.reserve(end_frame - start_frame);

            // 循环使用全局索引 i
            for (size_t i = start_frame; i < end_frame; ++i) { // <<< 这里使用全局索引 i
                double t_sec;
                t_obs->GetEngValue(i, t_sec); // <<< 传入全局索引 i
                qDebug()<<t_sec;
                time_mapping.push_back(static_cast<uint64_t>(t_sec * 1e9));
            }
            dg4->ClearData();


            // ⚡ 第二步：读取视频数据并直接写入文件
            // 保持与原始函数一致，直接用全局索引循环
            Reader.ReadPartialData(*dg4, start_frame, end_frame);
            auto v_obs = CreateChannelObserver(*dg4, *cg4, *cn_video);

            size_t time_map_idx = 0;
            // 循环使用全局索引 i
            for (size_t i = start_frame; i < end_frame; ++i) { // <<< 这里使用全局索引 i
                std::vector<uint8_t> bytes;
                v_obs->GetChannelValue(i, bytes); // <<< 传入全局索引 i

                // current_frame_global_idx 就是 i
                size_t current_frame_global_idx = i;

                // `time_mapping` 的索引应该与 `(i - start_frame)` 对应
                if (!bytes.empty() && (i - start_frame) < time_mapping.size()) { // 检查 time_mapping 索引
                    uint64_t current_relative_time_ns = time_mapping[i - start_frame]; // <<< 访问 time_mapping 时用局部索引

                    std::stringstream ss;
                    ss << "video_frame_" << std::setfill('0') << std::setw(8) << current_frame_global_idx
                       << "_ts_" << current_relative_time_ns << ".bin";
                    std::string filename_out = ss.str();

                    fs::path full_path = out_dir_path / filename_out;

                    std::ofstream outfile(full_path, std::ios::out | std::ios::binary);
                    if (outfile.is_open()) {
                        outfile.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
                        outfile.close();
                        total_saved_files++;
                    } else {
                        std::cerr << "Error: Failed to open/write file: " << full_path.string() << std::endl;
                    }

                    // time_map_idx++; // 不再需要 time_map_idx，直接用 (i - start_frame)
                } else {
                    if (current_frame_global_idx >= start_frame && current_frame_global_idx < end_frame) {
                        std::cerr << "Warning: Frame " << current_frame_global_idx << " from " << filename
                                  << " has empty video data or time mapping issue. Not saving to file." << std::endl;
                    }
                }
            }
            dg4->ClearData();
        }
    }

    Reader.Close();
    std::cout << "Video frame extraction complete. Saved " << total_saved_files << " BIN files to " << output_dir << std::endl;
    return total_saved_files;
}

std::vector<TimestampedData> readallmdftobin(std::string filename, double start, double end) {
    std::vector<TimestampedData> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();

    const auto* mdf_file = Reader.GetFile();
    const auto start_time_ns = mdf_file->Header()->StartTime();

    mdf::DataGroupList dg_list;
    mdf_file->DataGroups(dg_list);

    for (auto* dg4 : dg_list) {
        for (auto* cg4 : dg4->ChannelGroups()) {
            const auto* cn_t = cg4->GetChannel("t");
            const auto* cn_video = cg4->GetChannel("VideoRawdata_VC0");
            if (!cn_t || !cn_video) continue;

            auto t_obs = CreateChannelObserver(*dg4, *cg4, *cn_t);
            auto v_obs = CreateChannelObserver(*dg4, *cg4, *cn_video);

            size_t total_frames = v_obs->NofSamples();
            collected_data.reserve(total_frames);
            // std::cout<<total_frames<<endl;
            // std::cout<<t_obs->NofSamples()<<endl;


            auto [start_frame, end_frame] = TimeToFrameRange(start, end, total_frames);
            // std::cout<<start_frame<<endl;
            // std::cout<<end_frame<<endl;
            // ⚡ 第一步：读取时间通道
            Reader.ReadPartialData(*dg4,start_frame,end_frame);

            std::vector<uint64_t> time_mapping;
            //size_t n = t_obs->NofSamples();
            time_mapping.reserve(end_frame-start_frame+1);

            for (size_t i = start_frame; i < end_frame; ++i) {
                double t_sec ;
                t_obs->GetEngValue(i,t_sec); // 避免 string 转 double

                time_mapping.push_back(static_cast<uint64_t>(t_sec * 1e9));
            }
            /*
            uint64_t start_ns = static_cast<uint64_t>(start * 1e9);
            uint64_t end_ns   = static_cast<uint64_t>(end   * 1e9);

            auto start_it = std::lower_bound(time_mapping.begin(), time_mapping.end(), start_ns);
            auto end_it   = std::upper_bound(time_mapping.begin(), time_mapping.end(), end_ns);

            size_t start_idx = start_it - time_mapping.begin();
            size_t end_idx   = end_it   - time_mapping.begin();
            std::cout<<start_idx<<endl;
            std::cout<<end_idx<<endl;*/
            dg4->ClearData(); // 清理时间通道的数据，避免占内存

            // ⚡ 第二步：只读需要的视频区间

            // 每帧时长（秒）


            Reader.ReadPartialData(*dg4, start_frame, end_frame);
            size_t  timeindex=0;
            for (size_t i = start_frame; i < end_frame; ++i) {
                std::vector<uint8_t> bytes;
                v_obs->GetChannelValue(i, bytes);


                if (!bytes.empty()) {
                    collected_data.push_back(
                        {start_time_ns, time_mapping[timeindex], std::move(bytes), i});

                    timeindex++;

                }
            }

            dg4->ClearData(); // 清理视频数据，避免内存爆掉
        }
    }

    Reader.Close();
    return collected_data;
}

// std::vector<FrameMetadata> readallmdftobin_to_files(const std::string& filename,
//                                                     const std::string& output_dir,
//                                                     double start_sec,
//                                                     double end_sec,
//                                                     const std::string& video_channel_name) {
//     std::vector<FrameMetadata> collected_metadata;

//     mdf::MdfReader Reader(filename);
//     if (!Reader.IsOk()) { // 增加错误检查
//         qCritical() << "Error: Could not open MDF file:" << QString::fromStdString(filename);
//         return collected_metadata;
//     }
//     Reader.ReadEverythingButData();

//     const auto* mdf_file = Reader.GetFile();
//     const auto file_start_time_ns = mdf_file->Header()->StartTime();

//     fs::path out_dir_path(output_dir);
//     if (!fs::exists(out_dir_path) && !fs::create_directories(out_dir_path)) {
//         qCritical() << "Error: Failed to create output directory:" << QString::fromStdString(output_dir);
//         return collected_metadata;
//     }

//     mdf::DataGroupList dg_list;
//     mdf_file->DataGroups(dg_list);

//     for (auto* dg4 : dg_list) {
//         for (auto* cg4 : dg4->ChannelGroups()) {
//             const auto* cn_t = cg4->GetChannel("t");
//             const auto* cn_video = cg4->GetChannel(video_channel_name);
//             if (!cn_t || !cn_video) continue;

//             // 1. 创建持久观察者
//             auto t_obs = CreateChannelObserver(*dg4, *cg4, *cn_t);
//             auto v_obs = CreateChannelObserver(*dg4, *cg4, *cn_video);
//             size_t total_frames = v_obs->NofSamples();

//             if (total_frames == 0) continue;

//             // 2. 计算帧范围
//             auto [start_frame, end_frame] = TimeToFrameRange(start_sec, end_sec, total_frames);

//             if (start_frame >= end_frame) continue; // 检查范围有效性

//             size_t num_frames_to_read = end_frame - start_frame;

//             // ⚡ 单次读取：加载时间通道和视频通道数据
//             Reader.ReadPartialData(*dg4, start_frame, end_frame);

//             // 3. 提取时间戳
//             std::vector<uint64_t> time_mapping;
//             // 修正 reserve 大小
//             time_mapping.reserve(num_frames_to_read);

//             for (size_t i = start_frame; i < end_frame; ++i) {
//                 double t_sec;
//                 t_obs->GetEngValue(i, t_sec);
//                 // 移除调试输出
//                 // qDebug()<<t_sec;
//                 time_mapping.push_back(static_cast<uint64_t>(t_sec * 1e9));
//             }

//             // 4. 提取视频数据并写入文件
//             size_t timeindex = 0;
//             for (size_t i = start_frame; i < end_frame; ++i) {
//                 std::vector<uint8_t> bytes;
//                 // 使用原始观察者 v_obs 和绝对索引 i
//                 v_obs->GetChannelValue(i, bytes);

//                 // 确保 bytes 非空，且 time_mapping 索引有效
//                 if (!bytes.empty() && timeindex < time_mapping.size()) {
//                     uint64_t current_relative_time_ns = time_mapping[timeindex];

//                     // 文件名和路径
//                     std::stringstream ss;
//                     ss << "video_frame_" << std::setfill('0') << std::setw(8) << i << ".bin";
//                     fs::path full_path = out_dir_path / ss.str();

//                     std::ofstream outfile(full_path, std::ios::out | std::ios::binary);
//                     if (outfile.is_open()) {
//                         outfile.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
//                         outfile.close();

//                         // 收集元数据
//                         collected_metadata.push_back({
//                             file_start_time_ns,
//                             current_relative_time_ns,
//                             i,
//                             full_path.string()
//                         });
//                     } else {
//                         qCritical() << "Error: Failed to open/write file:" << QString::fromStdString(full_path.string());
//                     }
//                 }
//                 timeindex++;
//             }

//             // 仅在 DataGroup 完成后清理一次数据
//             dg4->ClearData();
//         }
//     }

//     Reader.Close();
//     qDebug() << "Video frame extraction complete. Collected" << collected_metadata.size() << "metadata entries.";
//     return collected_metadata;
// }
std::vector<FrameMetadata> readallmdftobin_to_files(const std::string& filename_qt,
                                                    const std::string& output_dir,
                                                    double start_sec,
                                                    double end_sec,
                                                    const std::string& video_channel_name) {
    std::vector<FrameMetadata> collected_metadata;

    QString tmpDir = QDir::tempPath() + "/mdf_tmp";
    QDir().mkpath(tmpDir);
    // QString tmpFile = tmpDir + "/input.mf4";
    QString tmpFile = tmpDir + "/" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".mf4";

    QFileInfo fi(QString::fromStdString(filename_qt));
    QString filename_qt_qstring = fi.absoluteFilePath();

    if (!QFile::exists(filename_qt_qstring)) {
        qCritical() << "Source MDF file does not exist:" << filename_qt_qstring;
        return collected_metadata;
    }

    if (QFile::exists(tmpFile)) QFile::remove(tmpFile);

    if (!QFile::copy(filename_qt_qstring, tmpFile)) {
        qCritical() << "Failed to copy MDF file to temporary path:" << tmpFile;
        return collected_metadata;
    }


    std::string filename = tmpFile.toLocal8Bit().toStdString();
    mdf::MdfReader Reader(filename);
    if (!Reader.IsOk()) { // 增加错误检查
        qCritical() << "Error: Could not open MDF file:" << QString::fromStdString(filename);
        return collected_metadata;
    }
    Reader.ReadEverythingButData();

    const auto* mdf_file = Reader.GetFile();
    const auto file_start_time_ns = mdf_file->Header()->StartTime();

    QString output_dir_qstring = QString::fromUtf8(output_dir.c_str());

    if (!QDir().exists(output_dir_qstring) && !QDir().mkpath(output_dir_qstring)) {
        qCritical() << "Error: Failed to create output directory:" << output_dir_qstring;
        return collected_metadata;
    }


    fs::path out_dir_path(output_dir_qstring.toLocal8Bit().toStdString());
    // if (!fs::exists(out_dir_path) && !fs::create_directories(out_dir_path)) {
    //     qCritical() << "Error: Failed to create output directory:" << QString::fromStdString(output_dir);
    //     return collected_metadata;
    // }

    mdf::DataGroupList dg_list;
    mdf_file->DataGroups(dg_list);

    for (auto* dg4 : dg_list) {
        for (auto* cg4 : dg4->ChannelGroups()) {
            const auto* cn_t = cg4->GetChannel("t");
            const auto* cn_video = cg4->GetChannel(video_channel_name);
            if (!cn_t || !cn_video) continue;

            // 1. 创建观察者并计算帧范围
            auto t_obs = CreateChannelObserver(*dg4, *cg4, *cn_t);
            auto v_obs = CreateChannelObserver(*dg4, *cg4, *cn_video);
            size_t total_frames = v_obs->NofSamples();

            if (total_frames == 0) continue;

            auto [start_frame, end_frame] = TimeToFrameRange(start_sec, end_sec, total_frames);
            if (start_frame >= end_frame) continue;

            size_t num_frames_to_read = end_frame - start_frame;

            // ----------------------------------------------------
            // ⚡ 优化: 快速检查模式
            // ----------------------------------------------------
            bool all_files_exist = true;
            std::vector<fs::path> expected_paths;
            expected_paths.reserve(num_frames_to_read);

            for (size_t i = start_frame; i < end_frame; ++i) {
                std::stringstream ss;
                ss << "video_frame_" << std::setfill('0') << std::setw(8) << i << ".bin";
                fs::path full_path = out_dir_path / ss.str();
                expected_paths.push_back(full_path);

                if (!fs::exists(full_path)) {
                    all_files_exist = false;
                    break;
                }
            }

            if (all_files_exist) {
                qDebug() << "All BIN files exist. Skipping video data read and write.";

                // 清除视频通道的观察者，只加载时间通道数据 (T通道数据通常很小)
                // 注意：这里需要假设 CreateChannelObserver 或 Reader 允许我们动态控制加载的数据。
                // 如果 Reader 依赖于 v_obs 存在来决定是否加载视频数据，则该方法有效。

                // 为了安全，我们只重新注册时间通道观察者（如果 API 支持）
                // 另一种方法是直接对 MdfReader::ReadPartialData() 调用一个只包含时间通道的 DG。
                // 这里我们假设 Reader.ReadPartialData 只会加载活跃观察者所需的数据：

                // 重新读取时间数据
                Reader.ReadPartialData(*dg4, start_frame, end_frame); // 仅加载 time channel data

                for (size_t i = 0; i < num_frames_to_read; ++i) {
                    size_t absolute_index = start_frame + i;

                    double t_sec;
                    // 提取时间戳
                    t_obs->GetEngValue(absolute_index, t_sec);
                    uint64_t current_relative_time_ns = static_cast<uint64_t>(t_sec * 1e9);

                    // 收集元数据
                    collected_metadata.push_back({
                        file_start_time_ns,
                        current_relative_time_ns,
                        absolute_index,
                        expected_paths[i].string() // 使用预计算的路径
                    });
                }

                dg4->ClearData(); // 清理加载的时间数据
                continue; // 成功快速返回这组数据，进入下一个 ChannelGroup
            }


            // ----------------------------------------------------
            // ⚡ 慢路径: 至少有一个文件丢失，必须执行完整的读取和写入流程
            // ----------------------------------------------------
            qDebug() << "Some BIN files are missing. Performing full data extraction.";

            // ⚡ 单次读取：加载时间通道和视频通道数据 (这是最耗时的步骤)
            Reader.ReadPartialData(*dg4, start_frame, end_frame);

            // 3. 提取时间戳并同步写入文件
            // 由于上面已经检查了 time_mapping 的大小，这里不再重新 reserve
            std::vector<uint64_t> time_mapping;
            time_mapping.reserve(num_frames_to_read);

            for (size_t i = start_frame; i < end_frame; ++i) {
                double t_sec;
                t_obs->GetEngValue(i, t_sec);
                time_mapping.push_back(static_cast<uint64_t>(t_sec * 1e9));
            }

            // 4. 提取视频数据并写入文件 (包含 per-frame 检查以防万一)
            size_t timeindex = 0;
            for (size_t i = start_frame; i < end_frame; ++i) {
                if (timeindex >= time_mapping.size()) break;

                uint64_t current_relative_time_ns = time_mapping[timeindex];

                // 文件名和路径
                std::stringstream ss;
                ss << "video_frame_" << std::setfill('0') << std::setw(8) << i << ".bin";
                fs::path full_path = out_dir_path / ss.str();

                // 检查文件是否已存在 (以防快速检查模式漏掉，或者部分文件丢失)
                if (!fs::exists(full_path)) {
                    std::vector<uint8_t> bytes;
                    v_obs->GetChannelValue(i, bytes);

                    if (bytes.empty()) {
                        qWarning() << "Frame" << i << "has empty data. Skipping write.";
                        timeindex++;
                        continue;
                    }

                    std::ofstream outfile(QString::fromStdString(full_path.string()).toLocal8Bit().toStdString(),
                                          std::ios::out | std::ios::binary);
                    if (outfile.is_open()) {
                        outfile.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
                        outfile.close();
                    } else {
                        qCritical() << "Error: Failed to open/write file:" << QString::fromStdString(full_path.string());
                        timeindex++;
                        continue;
                    }
                }

                // 收集元数据
                collected_metadata.push_back({
                    file_start_time_ns,
                    current_relative_time_ns,
                    i,
                    full_path.string()
                });

                timeindex++;
            }

            // 仅在 DataGroup 完成后清理一次数据
            dg4->ClearData();
        }
    }

    Reader.Close();
    if (QFile::exists(tmpFile)) {
        if (!QFile::remove(tmpFile)) {
            qWarning() << "Warning: Failed to remove temporary MF4 file:" << tmpFile;
        }
    }
    qDebug() << "Video frame extraction complete. Collected" << collected_metadata.size() << "metadata entries.";
    return collected_metadata;
}

std::vector<TimestampedCarSRSYawRATE> readallmf4CarSRSYawRate(std::string filename){

    std::vector<TimestampedCarSRSYawRATE> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();



    const auto* mdf_file = Reader.GetFile();

    const auto start_time_ns=mdf_file->Header()->StartTime();


    mdf::DataGroupList dg_list;

    mdf_file->DataGroups(dg_list);

    bool getdataflag=false;
    for (auto* dg4 : dg_list) {
        // Subscribers holds the sample data for a channel.
        // You should normally only subscribe on some channels.
        // We need a list to hold them.

        mdf::ChannelObserverList subscriber_list;
        const auto cg_list = dg4->ChannelGroups();
        for (const auto* cg4 : cg_list ) {

            if(cg_list.front()->Name()=="CCU_SRS_1"){
                getdataflag= true;
                const auto cn_list = cg4->Channels();
                for (const auto* cn4 : cn_list) {


                    // Create a subscriber and add it to the temporary list
                    auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
                    subscriber_list.emplace_back(std::move(sub));
                }
            }

        }
        // Now it is time to read in all samples
        Reader.ReadData(*dg4); // Read raw data from file








        std::vector<uint8_t> allbytes;

        allbytes.clear();

        std::vector<uint64_t> time_mapping;



        for (auto& obs : subscriber_list) {
            if (obs->Name() == "t") {
                size_t n = obs->NofSamples();
                time_mapping.resize(n);
                for (size_t sample = 0; sample < n; ++sample) {
                    auto relative_time_str = obs->EngValueToString(sample);
                    double relative_time_sec = std::stod(relative_time_str);
                    time_mapping[sample] = static_cast<uint64_t>(relative_time_sec * 1e9);
                }
            }
        }

        //std::ofstream outFile("output.bin", std::ios::binary);
        for (auto& obs : subscriber_list) {
            //std::cout<<obs->Name()<<endl;
            if (obs->Name() == "SRS_YawRate") {
                size_t n = obs->NofSamples();
                for (size_t sample = 0; sample < n; ++sample) {
                    //std::vector<uint8_t> bytes;
                    double YawRate;
                    obs->GetEngValue(sample,YawRate);

                    //auto vehicle_speed=obs->EngValueToString(sample);
                    if ( sample < time_mapping.size()) {
                        collected_data.push_back({start_time_ns, time_mapping[sample], YawRate,sample});
                    }
                }
            }
        }
        //outFile.write(reinterpret_cast<const char*>(allbytes.data()), allbytes.size());
        //outFile.close();

        //qDebug() << "Channel Value (hex):" << allbytes.size();

        // Not needed in this example as we delete the subscribers,
        // but it is good practise to remove samples data from memory
        // when it is no longer needed.
        dg4->ClearData();
    }
    Reader.Close(); // Close the file
    if(getdataflag){
        std::cout<<"GET Car Speed Success"<<endl;
    }
    return collected_data;

}

std::vector<TimestampedCarSRSYawRATE> readallmf4CarSRSYawRate8T8(std::string filename){

    std::vector<TimestampedCarSRSYawRATE> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();



    const auto* mdf_file = Reader.GetFile();

    const auto start_time_ns=mdf_file->Header()->StartTime();


    mdf::DataGroupList dg_list;

    mdf_file->DataGroups(dg_list);

    bool getdataflag=false;
    for (auto* dg4 : dg_list) {
        // Subscribers holds the sample data for a channel.
        // You should normally only subscribe on some channels.
        // We need a list to hold them.

        mdf::ChannelObserverList subscriber_list;
        const auto cg_list = dg4->ChannelGroups();
        for (const auto* cg4 : cg_list ) {

            if(cg_list.front()->Name()=="GW_SRS_3_A"){
                getdataflag= true;
                const auto cn_list = cg4->Channels();
                for (const auto* cn4 : cn_list) {


                    // Create a subscriber and add it to the temporary list
                    auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
                    subscriber_list.emplace_back(std::move(sub));
                }
            }

        }
        // Now it is time to read in all samples
        Reader.ReadData(*dg4); // Read raw data from file








        std::vector<uint8_t> allbytes;

        allbytes.clear();

        std::vector<uint64_t> time_mapping;



        for (auto& obs : subscriber_list) {
            if (obs->Name() == "t") {
                size_t n = obs->NofSamples();
                time_mapping.resize(n);
                for (size_t sample = 0; sample < n; ++sample) {
                    auto relative_time_str = obs->EngValueToString(sample);
                    double relative_time_sec = std::stod(relative_time_str);
                    time_mapping[sample] = static_cast<uint64_t>(relative_time_sec * 1e9);
                }
            }
        }

        //std::ofstream outFile("output.bin", std::ios::binary);
        for (auto& obs : subscriber_list) {
            //std::cout<<obs->Name()<<endl;
            if (obs->Name() == "SRS_YawRate") {
                size_t n = obs->NofSamples();
                for (size_t sample = 0; sample < n; ++sample) {
                    //std::vector<uint8_t> bytes;
                    double YawRate;
                    obs->GetEngValue(sample,YawRate);

                    //auto vehicle_speed=obs->EngValueToString(sample);
                    if ( sample < time_mapping.size()) {
                        collected_data.push_back({start_time_ns, time_mapping[sample], YawRate,sample});
                    }
                }
            }
        }
        //outFile.write(reinterpret_cast<const char*>(allbytes.data()), allbytes.size());
        //outFile.close();

        //qDebug() << "Channel Value (hex):" << allbytes.size();

        // Not needed in this example as we delete the subscribers,
        // but it is good practise to remove samples data from memory
        // when it is no longer needed.
        dg4->ClearData();
    }
    Reader.Close(); // Close the file
    if(getdataflag){
        std::cout<<"GET Car Speed Success"<<endl;
    }
    return collected_data;

}


std::vector<TimestampedCarData> readallmf4CarData(std::string filename){


    std::vector<TimestampedCarData> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();



    const auto* mdf_file = Reader.GetFile();

    const auto start_time_ns=mdf_file->Header()->StartTime();


    mdf::DataGroupList dg_list;

    mdf_file->DataGroups(dg_list);

    bool getdataflag=false;
    for (auto* dg4 : dg_list) {
        // Subscribers holds the sample data for a channel.
        // You should normally only subscribe on some channels.
        // We need a list to hold them.

        mdf::ChannelObserverList subscriber_list;
        const auto cg_list = dg4->ChannelGroups();
        for (const auto* cg4 : cg_list ) {

            if(cg_list.front()->Name()=="CCU_BCS_2_C"){
                getdataflag= true;
            const auto cn_list = cg4->Channels();
            for (const auto* cn4 : cn_list) {


                // Create a subscriber and add it to the temporary list
                auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
                subscriber_list.emplace_back(std::move(sub));
            }
        }

        }
        // Now it is time to read in all samples
        Reader.ReadData(*dg4); // Read raw data from file








        std::vector<uint8_t> allbytes;

        allbytes.clear();

        std::vector<uint64_t> time_mapping;



        for (auto& obs : subscriber_list) {
            if (obs->Name() == "t") {
                size_t n = obs->NofSamples();
                time_mapping.resize(n);
                for (size_t sample = 0; sample < n; ++sample) {
                    auto relative_time_str = obs->EngValueToString(sample);
                    double relative_time_sec = std::stod(relative_time_str);
                    time_mapping[sample] = static_cast<uint64_t>(relative_time_sec * 1e9);
                }
            }
        }

        //std::ofstream outFile("output.bin", std::ios::binary);
        for (auto& obs : subscriber_list) {
           // std::cout<<obs->Name()<<endl;
            if (obs->Name() == "BCS_VehSpd") {
                size_t n = obs->NofSamples();
                for (size_t sample = 0; sample < n; ++sample) {
                    //std::vector<uint8_t> bytes;
                    double speed;
                    obs->GetEngValue(sample,speed);
                    //auto vehicle_speed=obs->EngValueToString(sample);
                    if ( sample < time_mapping.size()) {
                        collected_data.push_back({start_time_ns, time_mapping[sample], speed,sample});
                    }
                }
            }
        }
        //outFile.write(reinterpret_cast<const char*>(allbytes.data()), allbytes.size());
        //outFile.close();

        //qDebug() << "Channel Value (hex):" << allbytes.size();

        // Not needed in this example as we delete the subscribers,
        // but it is good practise to remove samples data from memory
        // when it is no longer needed.
        dg4->ClearData();
    }
    Reader.Close(); // Close the file
    if(getdataflag){
    std::cout<<"GET Car Speed Success"<<endl;
    }
    return collected_data;

}

std::vector<TimestampedCarData> readallmf4CarData8T8(std::string filename){


    std::vector<TimestampedCarData> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();



    const auto* mdf_file = Reader.GetFile();

    const auto start_time_ns=mdf_file->Header()->StartTime();


    mdf::DataGroupList dg_list;

    mdf_file->DataGroups(dg_list);

    bool getdataflag=false;
    for (auto* dg4 : dg_list) {
        // Subscribers holds the sample data for a channel.
        // You should normally only subscribe on some channels.
        // We need a list to hold them.

        mdf::ChannelObserverList subscriber_list;
        const auto cg_list = dg4->ChannelGroups();
        for (const auto* cg4 : cg_list ) {

            if(cg_list.front()->Name()=="BCS_2_A"){
                getdataflag= true;
                const auto cn_list = cg4->Channels();
                for (const auto* cn4 : cn_list) {


                    // Create a subscriber and add it to the temporary list
                    auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
                    subscriber_list.emplace_back(std::move(sub));
                }
            }

        }
        // Now it is time to read in all samples
        Reader.ReadData(*dg4); // Read raw data from file








        std::vector<uint8_t> allbytes;

        allbytes.clear();

        std::vector<uint64_t> time_mapping;



        for (auto& obs : subscriber_list) {
            if (obs->Name() == "t") {
                size_t n = obs->NofSamples();
                time_mapping.resize(n);
                for (size_t sample = 0; sample < n; ++sample) {
                    auto relative_time_str = obs->EngValueToString(sample);
                    double relative_time_sec = std::stod(relative_time_str);
                    time_mapping[sample] = static_cast<uint64_t>(relative_time_sec * 1e9);
                }
            }
        }

        //std::ofstream outFile("output.bin", std::ios::binary);
        for (auto& obs : subscriber_list) {
            // std::cout<<obs->Name()<<endl;
            if (obs->Name() == "BCS_VehSpd") {
                size_t n = obs->NofSamples();
                for (size_t sample = 0; sample < n; ++sample) {
                    //std::vector<uint8_t> bytes;
                    double speed;
                    obs->GetEngValue(sample,speed);
                    //auto vehicle_speed=obs->EngValueToString(sample);
                    if ( sample < time_mapping.size()) {
                        collected_data.push_back({start_time_ns, time_mapping[sample], speed,sample});
                    }
                }
            }
        }
        //outFile.write(reinterpret_cast<const char*>(allbytes.data()), allbytes.size());
        //outFile.close();

        //qDebug() << "Channel Value (hex):" << allbytes.size();

        // Not needed in this example as we delete the subscribers,
        // but it is good practise to remove samples data from memory
        // when it is no longer needed.
        dg4->ClearData();
    }
    Reader.Close(); // Close the file
    if(getdataflag){
        std::cout<<"GET Car Speed Success"<<endl;
    }
    return collected_data;

}

std::vector<uint16_t> decodeRaw12(const std::vector<uint8_t>& data) {
    std::vector<uint16_t> pixels;

    size_t length = data.size();

    for (size_t i = 0; i + 2 < length; i += 3) {
        uint8_t b0 = data[i];
        uint8_t b1 = data[i + 1];
        uint8_t b2 = data[i + 2];

        uint16_t pixel0 = (b0 << 4) | (b2 & 0x0F);
        uint16_t pixel1 = (b1 << 4) | (b2 >> 4);

        pixels.push_back(pixel0);
        pixels.push_back(pixel1);
    }

    return pixels;
}


std::vector<uint8_t> extractRaw12FromFrame(const std::vector<uint8_t>& frame_bytes) {
    std::vector<uint8_t> raw12_payload;

    size_t total_len = frame_bytes.size();
    size_t step = 6178;

    // 遍历每个 5770 字节块
    for (size_t i = 0; i + step <= total_len; i += step) {
        // 取第 i+8 到 i+5768 字节（即 [i+8, i+5770-2)）
        raw12_payload.insert(raw12_payload.end(),
                             frame_bytes.begin() + i + 8,
                             frame_bytes.begin() + i + step - 2-24);
    }

    return raw12_payload;
}


std::vector<uint8_t> extractRaw12Frames(const std::vector<uint8_t>& frame_bytes, size_t offset, size_t step) {
    std::vector<uint8_t> raw12_payload;
    if (offset + step <= frame_bytes.size()) {
        raw12_payload.insert(raw12_payload.end(),
                             frame_bytes.begin() + offset,
                             frame_bytes.begin() + offset + step);
    }


    return extractRaw12FromFrame(raw12_payload);
}


std::vector<uint8_t> extractRaw12FromFrame8x8(const std::vector<uint8_t>& frame_bytes) {
    std::vector<uint8_t> raw12_payload;

    size_t total_len = frame_bytes.size();
    size_t step = 12346;

    // 遍历每个 5770 字节块
    for (size_t i = 0; i + step <= total_len; i += step) {
        // 取第 i+8 到 i+5768 字节（即 [i+8, i+5770-2)）
        raw12_payload.insert(raw12_payload.end(),
                             frame_bytes.begin() + i + 8,
                             frame_bytes.begin() + i + step - 2-48);
    }

    return raw12_payload;
}

std::vector<uint8_t> extractRaw12Frames8x8(const std::vector<uint8_t>& frame_bytes, size_t offset, size_t step) {
    std::vector<uint8_t> raw12_payload;
    if (offset + step <= frame_bytes.size()) {
        raw12_payload.insert(raw12_payload.end(),
                             frame_bytes.begin() + offset,
                             frame_bytes.begin() + offset + step);
    }


    return extractRaw12FromFrame8x8(raw12_payload);
}

/*
std::vector<TimestampedData> readallmdftobin(std::string filename, double start, double end){

    std::vector<TimestampedData> collected_data;
    mdf::MdfReader Reader(filename);
    Reader.ReadEverythingButData();




    const auto* mdf_file = Reader.GetFile();

    const auto start_time_ns=mdf_file->Header()->StartTime();



    mdf::DataGroupList dg_list;

    mdf_file->DataGroups(dg_list);



    for (auto* dg4 : dg_list) {
        // Subscribers holds the sample data for a channel.
        // You should normally only subscribe on some channels.
        // We need a list to hold them.
        std::cout<<"122"<<endl;

        mdf::ChannelObserverList subscriber_list;
        const auto cg_list = dg4->ChannelGroups();
        for (const auto* cg4 : cg_list ) {

            const auto cn_list = cg4->Channels();
            for (const auto* cn4 : cn_list) {


                // Create a subscriber and add it to the temporary list
                auto sub = CreateChannelObserver(*dg4, *cg4, *cn4);
                subscriber_list.emplace_back(std::move(sub));
            }
        }


        // Now it is time to read in all samples
        Reader.ReadData(*dg4); // Read raw data from file








        std::vector<uint8_t> allbytes;

        allbytes.clear();

        std::vector<uint64_t> time_mapping;

        size_t start_idxabout = static_cast<size_t>(start / 0.06);
        size_t end_idxabout   = static_cast<size_t>(end / 0.04);
        std::cout <<"end_idxabout"<<end_idxabout<<endl;

        for (auto& obs : subscriber_list) {
            if (obs->Name() == "t") {

                size_t n = obs->NofSamples();

                if (start_idxabout >= n) start_idxabout = n - 1;
                if (end_idxabout > n) end_idxabout = n;

                time_mapping.resize(n);
                std::cout <<"n"<<n<<endl;
                for (size_t sample = 0; sample < n; ++sample) {
                    auto relative_time_str = obs->EngValueToString(sample);
                    double relative_time_sec = std::stod(relative_time_str);

                    time_mapping[sample] = static_cast<uint64_t>(relative_time_sec * 1e9);
                }
            }
        }

        uint64_t start_ns = static_cast<uint64_t>(start * 1e9);
        uint64_t end_ns   = static_cast<uint64_t>(end * 1e9);


        auto start_it = std::lower_bound(time_mapping.begin(), time_mapping.end(), start_ns);
        auto end_it   = std::upper_bound(time_mapping.begin(), time_mapping.end(), end_ns);

        if(start<=0 && end<=0){



        }
        size_t start_idx = start_it - time_mapping.begin();
        size_t end_idx   = end_it - time_mapping.begin();

        if(start<=0 && end<=0){
            start_idx=0;
            end_idx =time_mapping.size();

        }
        std::cout <<"start_idx"<<start_idx<<endl;
        std::cout<<"end_idx"<<end_idx<<endl;
        //std::ofstream outFile("output.bin", std::ios::binary);
        for (auto& obs : subscriber_list) {
            if (obs->Name() == "VideoRawdata_VC0") {
                size_t n = obs->NofSamples();
                for (size_t sample = start_idx; sample < end_idx; ++sample) {
                    std::vector<uint8_t> bytes;
                    obs->GetChannelValue(sample, bytes);
                    if (!bytes.empty() && sample < time_mapping.size()) {

                        collected_data.push_back({start_time_ns, time_mapping[sample], bytes,sample});

                    }
                }
            }
        }
        //outFile.write(reinterpret_cast<const char*>(allbytes.data()), allbytes.size());
        //outFile.close();

        //qDebug() << "Channel Value (hex):" << allbytes.size();

        // Not needed in this example as we delete the subscribers,
        // but it is good practise to remove samples data from memory
        // when it is no longer needed.
        dg4->ClearData();
    }
    Reader.Close(); // Close the file

    return collected_data;

}


*/
