#ifndef LATENCYMONITORWIDGET_H
#define LATENCYMONITORWIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "LatencyMonitor.h"
#include <QVBoxLayout>  // 🚀 修复 C2061 错误的关键
#include <QPen>         // 用于设置曲线颜色
#include <QPainter>     // 用于设置渲染抗锯齿
#include <QLabel>

class LatencyMonitorWidget : public QWidget {
    Q_OBJECT
public:
    explicit LatencyMonitorWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Radar Latency Monitor");
        resize(800, 450);

        // 创建四条曲线
        seriesP = new QtCharts::QLineSeries(); seriesP->setName("Parsing");
        seriesS = new QtCharts::QLineSeries(); seriesS->setName("Signal Lag");
        seriesR = new QtCharts::QLineSeries(); seriesR->setName("Rendering");
        seriesT = new QtCharts::QLineSeries(); seriesT->setName("Total E2E");

        // 设置颜色和样式
        seriesT->setPen(QPen(Qt::red, 3));      // 总延迟用粗红线
        seriesP->setPen(QPen(Qt::green, 1));
        seriesS->setPen(QPen(Qt::blue, 1));
        seriesR->setPen(QPen(Qt::yellow, 1));

        chart = new QtCharts::QChart();
        chart->addSeries(seriesP); chart->addSeries(seriesS);
        chart->addSeries(seriesR); chart->addSeries(seriesT);
        chart->setTitle("Real-time Latency Breakdown (ms)");
        chart->setAnimationOptions(QtCharts::QChart::NoAnimation); // 禁用动画提高实时性

        axisX = new QtCharts::QValueAxis();
        axisX->setRange(0, 100);
        axisX->setLabelFormat("%d");
        axisX->setTitleText("Frame Count");

        axisY = new QtCharts::QValueAxis();
        axisY->setRange(0, 80); // 初始设为80ms
        axisY->setTitleText("Time (ms)");

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);

        seriesP->attachAxis(axisX); seriesP->attachAxis(axisY);
        seriesS->attachAxis(axisX); seriesS->attachAxis(axisY);
        seriesR->attachAxis(axisX); seriesR->attachAxis(axisY);
        seriesT->attachAxis(axisX); seriesT->attachAxis(axisY);

        QtCharts::QChartView *cv = new QtCharts::QChartView(chart);
        cv->setRenderHint(QPainter::Antialiasing);

        dropRateLabel = new QLabel("Drop Rate: 0.00% (Lost: 0)", this);
        dropRateLabel->setStyleSheet("font-weight: bold; color: darkred; font-size: 14px;");


        auto *lyt = new QVBoxLayout(this);
        lyt->addWidget(dropRateLabel); // 🚀 把标签加在图表上方
        lyt->addWidget(cv);

        connect(&LatencyMonitor::instance(), &LatencyMonitor::dropRateUpdated,
                this, &LatencyMonitorWidget::onDropRateUpdated);

        connect(&LatencyMonitor::instance(), &LatencyMonitor::newLatencyReport,
                this, &LatencyMonitorWidget::updateData);
    }

    void clearChart() {
        seriesP->clear();
        seriesS->clear();
        seriesR->clear();
        seriesT->clear();
        count = 0; // 计数器归零
        axisX->setRange(0, 100); // 坐标轴重置
        qDebug() << "Latency Chart cleared.";
    }

private slots:
    void updateData(double p, double s, double r, double t) {
        seriesP->append(count, p);
        seriesS->append(count, s);
        seriesR->append(count, r);
        seriesT->append(count, t);
        count++;

        // 滑动窗口：只看最近100帧
        if (seriesT->count() > 100) {
            seriesP->remove(0); seriesS->remove(0);
            seriesR->remove(0); seriesT->remove(0);
            axisX->setRange(count - 100, count);
        }

        // 动态调整 Y 轴上限
        if (t > axisY->max()) {
            axisY->setRange(0, t + 20);
        }
    }
    void onDropRateUpdated(double rate, uint32_t totalDropped,uint32_t totalFrames, uint64_t overflow) {
        QString status = QString("Drop Rate: %1% (Lost: %2 / Total: %3) | Queue Overflow: %4")
        .arg(rate, 0, 'f', 2)
            .arg(totalDropped)
            .arg(totalFrames)
            .arg(overflow); // 🚀 显示溢出数

        dropRateLabel->setText(status);

        // 工业级警示：如果丢帧率超过 1%，变红提醒
        if (rate > 1.0 || overflow > 0) {
            // 如果有溢出，背景变橘黄色提醒硬件/处理压力
            QString style = (overflow > 0) ? "background-color: #FFCC00; color: black;" : "background-color: yellow; color: red;";
            dropRateLabel->setStyleSheet("font-weight: bold; " + style);
        } else {
            dropRateLabel->setStyleSheet("font-weight: bold; color: darkgreen;");
        }
    }

private:
    QtCharts::QLineSeries *seriesP, *seriesS, *seriesR, *seriesT;
    QtCharts::QValueAxis *axisX, *axisY;
    QtCharts::QChart *chart;
    int count = 0;
    QLabel *dropRateLabel;
};

#endif
