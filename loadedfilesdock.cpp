#include "LoadedFilesDock.h"
#include <QFileInfo>
#include <QFormLayout>
LoadedFilesDock::LoadedFilesDock(QWidget* parent)
    : QDockWidget("Loaded Files", parent)
{
    setObjectName("LoadedFilesDock");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

    // ✅ 先创建 QLabel
    pointBinLabel    = new QLabel("N/A");
    videoLabel       = new QLabel("N/A");
    carDataMF4Label  = new QLabel("N/A");
    radarMF4Label    = new QLabel("N/A");
    adcBinLabel      = new QLabel("N/A");
    onePCDLabel      = new QLabel("N/A");

    QWidget* container = new QWidget(this);
    QFormLayout* layout = new QFormLayout(container);
    layout->setContentsMargins(5,5,5,5);

    // ✅ 使用已经创建的 QLabel 添加到布局
    layout->addRow("Point Bin", pointBinLabel);
    layout->addRow("Video", videoLabel);
    layout->addRow("CarData MF4", carDataMF4Label);
    layout->addRow("Radar MF4", radarMF4Label);
    layout->addRow("ADC Bin", adcBinLabel);
    layout->addRow("One PCD", onePCDLabel);

    setWidget(container);
}

// 辅助函数
void LoadedFilesDock::updateLabel(QLabel* label, const QString& file, const QString& prefix)
{
    if (!file.isEmpty()) {
        label->setText(QFileInfo(file).fileName());              // 只显示文件名
        label->setToolTip(QFileInfo(file).absoluteFilePath());   // 鼠标悬停显示完整路径
    } else {
        label->setText("N/A");
        label->setToolTip("");
    }
}

// 各类文件更新接口
void LoadedFilesDock::updatePointBin(const QString& file)   { updateLabel(pointBinLabel, file, "Point Bin"); }
void LoadedFilesDock::updateVideo(const QString& file)      { updateLabel(videoLabel, file, "Video"); }
void LoadedFilesDock::updateCarDataMF4(const QString& file) { updateLabel(carDataMF4Label, file, "CarData MF4"); }
void LoadedFilesDock::updateRadarMF4(const QString& file)   { updateLabel(radarMF4Label, file, "Radar MF4"); }
void LoadedFilesDock::updateADCBin(const QString& file)     { updateLabel(adcBinLabel, file, "ADC Bin"); }
void LoadedFilesDock::updateOnePCD(const QString& file)     { updateLabel(onePCDLabel, file, "One PCD"); }
