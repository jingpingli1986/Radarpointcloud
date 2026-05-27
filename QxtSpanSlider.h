#ifndef QXTSPANSLIDER_H
#define QXTSPANSLIDER_H

#include <QSlider>

// 前向声明私有实现类
class QxtSpanSliderPrivate;

// QxtSpanSlider 类继承自 QSlider
class QxtSpanSlider : public QSlider {
    Q_OBJECT

    // QXT_DECLARE_PRIVATE(QxtSpanSlider) // 如果使用PIMPL手法来实现私有数据，可以取消注释这行

    // 属性声明，用于集成 Qt 的属性系统
    Q_PROPERTY(int lowerValue READ lowerValue WRITE setLowerValue)
    Q_PROPERTY(int upperValue READ upperValue WRITE setUpperValue)
    Q_PROPERTY(int lowerPosition READ lowerPosition WRITE setLowerPosition)
    Q_PROPERTY(int upperPosition READ upperPosition WRITE setUpperPosition)
    Q_PROPERTY(HandleMovementMode handleMovementMode READ handleMovementMode WRITE setHandleMovementMode)
    Q_ENUMS(HandleMovementMode) // 声明 HandleMovementMode 枚举类型

public:
    // 构造函数
    explicit QxtSpanSlider(QWidget* parent = 0);
    explicit QxtSpanSlider(Qt::Orientation orientation, QWidget* parent = 0);
    virtual ~QxtSpanSlider(); // 析构函数

    // 枚举：定义滑块柄移动模式
    enum HandleMovementMode {
        FreeMovement,   // 自由移动
        NoCrossing,     // 不交叉
        NoOverlapping   // 不重叠
    };

    // 枚举：定义滑块柄
    enum SpanHandle {
        NoHandle,       // 无柄
        LowerHandle,    // 下柄
        UpperHandle     // 上柄
    };

    // 获取和设置滑块柄移动模式
    HandleMovementMode handleMovementMode() const;
    void setHandleMovementMode(HandleMovementMode mode);

    // 获取下限和上限值
    int lowerValue() const;
    int upperValue() const;

    // 获取下限和上限位置
    int lowerPosition() const;
    int upperPosition() const;

public Q_SLOTS:
    // 设置值和位置的槽函数
    void setLowerValue(int lower);
    void setUpperValue(int upper);
    void setSpan(int lower, int upper);

    void setLowerPosition(int lower);
    void setUpperPosition(int upper);

Q_SIGNALS:
    // 范围和值变化的信号
    void spanChanged(int lower, int upper);
    void lowerValueChanged(int lower);
    void upperValueChanged(int upper);

    // 位置变化的信号
    void lowerPositionChanged(int lower);
    void upperPositionChanged(int upper);

    // 滑块柄按下的信号
    void sliderPressed(QxtSpanSlider::SpanHandle handle);

protected:
    // 事件处理函数：键盘、鼠标和绘制事件
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private:
    QxtSpanSliderPrivate* d_ptr; // 指向私有实现的指针
    friend class QxtSpanSliderPrivate; // 允许私有实现类访问 QxtSpanSlider 的私有成员
};

#endif // QXTSPANSLIDER_H
