#ifndef LOADEDFILESDOCK_H
#define LOADEDFILESDOCK_H

#include <QDockWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class LoadedFilesDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit LoadedFilesDock(QWidget* parent = nullptr);

    // 更新各类文件
    void updatePointBin(const QString& file);
    void updateVideo(const QString& file);
    void updateCarDataMF4(const QString& file);
    void updateRadarMF4(const QString& file);
    void updateADCBin(const QString& file);
    void updateOnePCD(const QString& file);

private:
    QLabel* pointBinLabel;
    QLabel* videoLabel;
    QLabel* carDataMF4Label;
    QLabel* radarMF4Label;
    QLabel* adcBinLabel;
    QLabel* onePCDLabel;

    // 辅助函数
    void updateLabel(QLabel* label, const QString& file, const QString& prefix);
};

#endif // LOADEDFILESDOCK_H
