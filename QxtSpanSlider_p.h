#ifndef QXTSPANSLIDER_P_H
#define QXTSPANSLIDER_P_H

#include <QStyle>
#include <QObject>
#include "QxtSpanSlider.h"

// 前向声明类
QT_FORWARD_DECLARE_CLASS(QStylePainter)
QT_FORWARD_DECLARE_CLASS(QStyleOptionSlider)

// QxtSpanSliderPrivate 类继承自 QObject
class QxtSpanSliderPrivate : public QObject {
    Q_OBJECT
public:
    // 构造函数
    QxtSpanSliderPrivate();

    // 初始化样式选项
    void initStyleOption(QStyleOptionSlider* option, QxtSpanSlider::SpanHandle handle = QxtSpanSlider::UpperHandle) const;

    // 根据方向获取点的位置
    int pick(const QPoint& pt) const
    {
        return q_ptr->orientation() == Qt::Horizontal ? pt.x() : pt.y();
    }

    // 将像素位置转换为范围值
    int pixelPosToRangeValue(int pos) const;

    // 处理鼠标按下事件
    void handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, QxtSpanSlider::SpanHandle handle);

    // 绘制滑块柄
    void drawHandle(QStylePainter* painter, QxtSpanSlider::SpanHandle handle) const;

    // 设置画笔
    void setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const;

    // 绘制跨度
    void drawSpan(QStylePainter* painter, const QRect& rect) const;

    // 触发滑动条动作
    void triggerAction(QAbstractSlider::SliderAction action, bool main);

    // 交换控制
    void swapControls();

    // 成员变量
    int lower;
    int upper;
    int lowerPos;
    int upperPos;
    int offset;
    int position;
    QxtSpanSlider::SpanHandle lastPressed;
    QxtSpanSlider::SpanHandle mainControl;
    QStyle::SubControl lowerPressed;
    QStyle::SubControl upperPressed;
    QxtSpanSlider::HandleMovementMode movement;
    bool firstMovement;
    bool blockTracking;

public Q_SLOTS:
    // 更新范围
    void updateRange(int min, int max);

    // 移动按下的滑块柄
    void movePressedHandle();

private:
    // 指向 QxtSpanSlider 的指针
    QxtSpanSlider* q_ptr;

    // 友元类
    friend class QxtSpanSlider;
};

#endif // QXTSPANSLIDER_P_H
