#ifndef RADAR_PROTOCOL_H
#define RADAR_PROTOCOL_H

#include <QtGlobal>

#pragma pack(push, 1)

// PCAP 全局头 (24字节)
struct PcapGlobalHeader {
    quint32 magic_number;
    quint16 version_major;
    quint16 version_minor;
    qint32  thiszone;
    quint32 sigfigs;
    quint32 snaplen;
    quint32 network;
};

// PCAP 数据包头 (16字节)
struct PcapPacketHeader {
    quint32 ts_sec;
    quint32 ts_usec;
    quint32 incl_len;
    quint32 orig_len;
};

// 雷达 Packet Header (32字节)
struct RadarPacketHeader {
    uint16_t e2eLength;      // 2字节
    uint16_t e2eCounter;     // 2字节
    uint32_t e2eDataId;      // 4字节
    uint32_t e2eCrc;         // 4字节
    uint32_t frameStartMagic; // 4字节 (0x00BE0001)
    uint16_t frameID;        // 2字节
    uint8_t  verPart1;       // 1字节
    uint8_t  verPart2;       // 1字节
    uint16_t packetID;       // 2字节
    uint16_t framePacketNum; // 2字节
    uint32_t packetByteSize; // 4字节
    uint32_t totalByteSize;  // 4字节
    uint32_t sectionType;    // 4字节
    uint32_t reserved;       // 4字节
}; // 合计：40 字节

// 雷达 Frame Header (42字节) - 仅在 packetID == 0 时出现
struct RadarFrameHeader {
    // 1. 固定标识符 (备用)
    uint16_t syncWord[4];             // 8字节: {0x0102, 0x0304, 0x0506, 0x0708}

    // 2. 帧信息
    uint32_t frameId;                 // 4字节: Frame ID 标识
    uint32_t dspId;                   // 4字节: 处理器标识 (备用)
    uint32_t reserved_a;              // 4字节: 预留 A

    // 3. 数据量统计
    uint32_t totalOutputSize;         // 4字节: 当前 Frame 输出的总字节数 (不含本 Header)
    uint32_t detectionPointListCount; // 4字节: 雷达检测到的点云个数
    uint32_t objectDataListCount;     // 4字节: 雷达输出的跟踪航迹个数
    uint32_t numObjDetectedByCfar;    // 4字节: CFAR 检测出的 object 个数

    // 4. 时间戳信息
    uint32_t timestampNanoseconds;    // 4字节: 时间戳 (单位: ns)
    uint32_t timestampSecondsLow32bit;// 4字节: 秒计数的低 32 位
    uint16_t timestampSecondsHigh16bit;// 2字节: 秒计数的高 16 位 (起始时间 1970-01-01)
    uint8_t  timestampSyncStatus;     // 1字节: 时间同步有效标志
    uint8_t  reserved_align_A;        // 1字节: 预留数据对齐 Byte

    // 5. 版本与模式信息
    uint32_t dss_or_tss_version;      // 4字节: 点云时为 DSS 版本, 航迹时为 TSS 版本
    uint32_t sp_version;              // 4字节: SP (信号处理) 版本
    uint32_t mss_version;             // 4字节: MSS (ARM1核) 版本
    uint32_t out_mode;                // 4字节: 输出的数据类型
    uint32_t sectionType;             // 4字节: 该 Section 数据类型

    // 6. 延迟信息
    uint32_t latency;                 // 4字节: 时延延迟 (以发波时间开始计算)
};


// egoInfo (input) - 约 63 字节
struct RadarEgoInfo {
    float egoSpeed;                  // 车速
    float egoYawRate;                // 偏航角速率
    float egoVehLongAccel;           // 自车纵向加速度
    float egoVehLateralAccel;        // 自车横向加速度
    float egoVehLongAccelOffsetValue;
    float egoVehLateralAccelOffsetValue;
    float egoYawRateOffsetValue;
    float egoFLWheelSpd;             // 左前轮速
    float egoFRWheelSpd;             // 右前轮速
    float egoRLWheelSpd;             // 左后轮速
    float egoRRWheelSpd;             // 右后轮速
    // float egoSteeringAngle_E2E;      // 转向角度
    // float egoSteeringAngleSpd_E2E;   // 转向角速度
    uint64_t timestampglobal;
    uint8_t currentGearLev;          // 档位
    uint8_t flWheelRotatedDirection; // 左前转向
    uint8_t frWheelRotatedDirection;
    uint8_t rlWheelRotatedDirection;
    uint8_t rrWheelRotatedDirection;
    uint8_t fWiperSwSt;              // 雨刮开关
    uint8_t tarLvl;                  // 悬架高度
    uint8_t heightLvl;               // 实际高度
    uint8_t reserved[4];
};

// radarMtnInfo (input_output) - 56 字节
struct RadarMtnInfo {
    float longOffset;        // 纵向安装位置
    float latOffset;         // 横向安装位置
    float mountHeight;       // 安装高度
    float rollAngle;         // 偏侧角
    float mountYaw;          // 方位安装角度
    float mountPitch;        // 俯仰安装角度
    float mountRoll;         // 翻滚安装角度
    float mountR;            // 到后轴中心距离
    float mountSigma;        // 夹角
    float defaultYawAngle;   // 理论方位角
    float defaultPitchAngle; // 理论俯仰角
    int8_t installDirection; // 安装方式
    uint8_t reserved[7];
};

// detObjList (output) - 数组结构 (SoA)
// 注意：该结构体包含大量 2048 长度的数组，总大小超过 300KB
struct RadarDetObjList {
    uint16_t detObjNum;      // 点迹目标数量
    uint16_t reserved2;
    uint8_t  aziQly[2048];   // 方位角质量
    uint8_t  eleQly[2048];   // 仰角质量
    uint8_t  dvQly[2048];    // 速度质量
    uint8_t  measStatus[2048]; // 点云监控指标
    uint8_t  existProb[2048];  // 存在概率
    uint8_t  pointStatus[2048];// 状态标识
    uint8_t  pointDoaStatus[2048];
    uint8_t  crossPathPowerRatio[2048];
    int8_t   velambTimes[2048];
    uint16_t velBin[2048];
    uint16_t rangeBin[2048];
    uint16_t aziBin[2048];
    uint16_t eleBin[2048];
    float    range[2048];     // 径向距离
    float    rangeVar[2048];  // 距离方差
    float    dopplerSpeed[2048]; // 相对径向速度
    float    velVar[2048];    // 速度方差
    float    rvCov[2048];
    float    azimuth[2048];   // 方位角
    float    azimuthVar[2048];
    float    elevation[2048]; // 俯仰角
    float    elevationVar[2048];
    float    snrdB[2048];     // 信噪比
    float    rcsdB[2048];     // RCS
    float    xPos[2048];      // 纵向位置 (真正需要的 X)
    float    yPos[2048];      // 横向位置 (真正需要的 Y)
    float    zPos[2048];      // 高度 (真正需要的 Z)
    float    radVelAbs[2048]; // 绝对径向速度
    float    powerdB[2048];   // 信号强度
    uint8_t  paddingData[2048 * 9];
};


// radarPara (input_output) - 452 字节
struct RadarParaInfo {
    uint8_t  radarId;                   // 雷达编号
    uint8_t  idleTimeId;                // idleTime控制每帧chirp周期
    uint8_t  waveSwitchId;              // 0:远波, 1:近波
    uint8_t  interferenceStateConf;     // 雷达干扰指示
    float    timestamp[100];            // 软件运行时间监控变量 (400字节)
    float    rResolutionC;              // 距离分辨率 (chirp带宽)
    float    rResolutionS;              // 距离分辨率 (跳频带宽)
    float    vResolution;               // 速度分辨率
    float    rangeMaxlim;               // 最大距离
    float    rangeMinlim;               // 最小距离
    uint32_t frameCount;                // 雷达帧号
    float    unAmbVelocityMin;          // 单帧不模糊速度min
    float    unAmbVelocityMax;          // 单帧不模糊速度max
    float    ambVelocityMin;            // 帧间解速度模糊值min
    float    ambVelocityMax;            // 帧间解速度模糊值max
    uint8_t  reserved[4];               // 保留位
};

struct RadarRDMap {
    uint16_t rdData[524288];           // 雷达通道积累dB值
};

// 最后的保留位
struct RadarReservedEnd {
    uint8_t reserved[2097152];         // 2MB 保留位
};

#pragma pack(pop)

#endif
