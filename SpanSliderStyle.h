#ifndef SPANSLIDERSTYLE_H
#define SPANSLIDERSTYLE_H
#include <QProxyStyle>
#include <QStyleOptionSlider>
#include <QPainter>
#include <QWidget>

class SpanSliderStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;

    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const override
    {
        if (control == CC_Slider) {
            if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {

                // 绘制槽
                QProxyStyle::drawComplexControl(control, option, painter, widget);

                // 获取两个 handle 的位置（QxtSpanSlider 内部 low/high handle）
                QRect rectLow = slider->subControls & SC_SliderHandle ? subControlRect(CC_Slider, slider, SC_SliderHandle, widget) : QRect();
                QRect rectHigh = rectLow; // 可以通过 QxtSpanSlider 的 API 获取 high handle rect

                // 绘制 lowHandle
                painter->setBrush(QColor("#009688"));
                painter->setPen(QColor("#222"));
                painter->drawEllipse(rectLow);

                // 绘制 highHandle
                painter->setBrush(QColor("#009688"));
                painter->setPen(QColor("#222"));
                painter->drawEllipse(rectHigh);
            }
            return;
        }
        QProxyStyle::drawComplexControl(control, option, painter, widget);
    }
};

#endif // SPANSLIDERSTYLE_H
