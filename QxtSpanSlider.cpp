#include "QxtSpanSlider.h"
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

#include "QxtSpanSlider_p.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QStylePainter>

QxtSpanSliderPrivate::QxtSpanSliderPrivate() :
        lower(0),
        upper(0),
        lowerPos(0),
        upperPos(0),
        offset(0),
        position(0),
        lastPressed(QxtSpanSlider::NoHandle),
        mainControl(QxtSpanSlider::LowerHandle),
        lowerPressed(QStyle::SC_None),
        upperPressed(QStyle::SC_None),
        movement(QxtSpanSlider::FreeMovement),
        firstMovement(false),
        blockTracking(false)
{
}

void QxtSpanSliderPrivate::initStyleOption(QStyleOptionSlider* option, QxtSpanSlider::SpanHandle handle) const
{
    const QxtSpanSlider* p = q_ptr;
    p->initStyleOption(option);
    option->sliderPosition = (handle == QxtSpanSlider::LowerHandle ? lowerPos : upperPos);
    option->sliderValue = (handle == QxtSpanSlider::LowerHandle ? lower : upper);
}

int QxtSpanSliderPrivate::pixelPosToRangeValue(int pos) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    int sliderMin = 0;
    int sliderMax = 0;
    int sliderLength = 0;
    const QSlider* p = q_ptr;
    const QRect gr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (p->orientation() == Qt::Horizontal)
    {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    }
    else
    {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::sliderValueFromPosition(p->minimum(), p->maximum(), pos - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

void QxtSpanSliderPrivate::handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, QxtSpanSlider::SpanHandle handle)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    QxtSpanSlider* p = q_ptr;
    const QStyle::SubControl oldControl = control;
    control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (control == QStyle::SC_SliderHandle)
    {
        position = value;
        offset = pick(pos - sr.topLeft());
        lastPressed = handle;
        p->setSliderDown(true);
        emit p->sliderPressed(handle);
    }
    if (control != oldControl)
        p->update(sr);
}

void QxtSpanSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const
{
    QColor highlight = q_ptr->palette().color(QPalette::Highlight);
    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0, highlight.darker(120));
    gradient.setColorAt(1, highlight.lighter(108));
    painter->setBrush(gradient);

    if (orientation == Qt::Horizontal)
        painter->setPen(QPen(highlight.darker(130), 0));
    else
        painter->setPen(QPen(highlight.darker(130), 0));
}
/*
void QxtSpanSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    const QSlider* p = q_ptr;

    // area
    QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    if (opt.orientation == Qt::Horizontal)
        groove.adjust(0, 0, -1, 0);
    else
        groove.adjust(0, 0, 0, -1);

    // pen & brush
    painter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));
    if (opt.orientation == Qt::Horizontal)
        setupPainter(painter, opt.orientation, groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
    else
        setupPainter(painter, opt.orientation, groove.left(), groove.center().y(), groove.right(), groove.center().y());

    // draw groove
    painter->drawRect(rect.intersected(groove));
}

void QxtSpanSliderPrivate::drawHandle(QStylePainter* painter, QxtSpanSlider::SpanHandle handle) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    opt.subControls = QStyle::SC_SliderHandle;
    QStyle::SubControl pressed = (handle == QxtSpanSlider::LowerHandle ? lowerPressed : upperPressed);
    if (pressed == QStyle::SC_SliderHandle)
    {
        opt.activeSubControls = pressed;
        opt.state |= QStyle::State_Sunken;
    }
    painter->drawComplexControl(QStyle::CC_Slider, opt);
}
*/

void QxtSpanSliderPrivate::triggerAction(QAbstractSlider::SliderAction action, bool main)
{
    int value = 0;
    bool no = false;
    bool up = false;
    const int min = q_ptr->minimum();
    const int max = q_ptr->maximum();
    const QxtSpanSlider::SpanHandle altControl = (mainControl == QxtSpanSlider::LowerHandle ? QxtSpanSlider::UpperHandle : QxtSpanSlider::LowerHandle);

    blockTracking = true;

    switch (action)
    {
    case QAbstractSlider::SliderSingleStepAdd:
        if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
        {
            value = qBound(min, upper + q_ptr->singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower + q_ptr->singleStep(), max);
        break;
    case QAbstractSlider::SliderSingleStepSub:
        if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
        {
            value = qBound(min, upper - q_ptr->singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower - q_ptr->singleStep(), max);
        break;
    case QAbstractSlider::SliderToMinimum:
        value = min;
        if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderToMaximum:
        value = max;
        if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderMove:
        if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
            up = true;
    case QAbstractSlider::SliderNoAction:
        no = true;
        break;
    default:
        qWarning("QxtSpanSliderPrivate::triggerAction: Unknown action");
        break;
    }

    if (!no && !up)
    {
        if (movement == QxtSpanSlider::NoCrossing)
            value = qMin(value, upper);
        else if (movement == QxtSpanSlider::NoOverlapping)
            value = qMin(value, upper - 1);

        if (movement == QxtSpanSlider::FreeMovement && value > upper)
        {
            swapControls();
            q_ptr->setUpperPosition(value);
        }
        else
        {
            q_ptr->setLowerPosition(value);
        }
    }
    else if (!no)
    {
        if (movement == QxtSpanSlider::NoCrossing)
            value = qMax(value, lower);
        else if (movement == QxtSpanSlider::NoOverlapping)
            value = qMax(value, lower + 1);

        if (movement == QxtSpanSlider::FreeMovement && value < lower)
        {
            swapControls();
            q_ptr->setLowerPosition(value);
        }
        else
        {
            q_ptr->setUpperPosition(value);
        }
    }

    blockTracking = false;
    q_ptr->setLowerValue(lowerPos);
    q_ptr->setUpperValue(upperPos);
}

void QxtSpanSliderPrivate::swapControls()
{
    qSwap(lower, upper);
    qSwap(lowerPressed, upperPressed);
    lastPressed = (lastPressed == QxtSpanSlider::LowerHandle ? QxtSpanSlider::UpperHandle : QxtSpanSlider::LowerHandle);
    mainControl = (mainControl == QxtSpanSlider::LowerHandle ? QxtSpanSlider::UpperHandle : QxtSpanSlider::LowerHandle);
}

void QxtSpanSliderPrivate::updateRange(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    // setSpan() takes care of keeping span in range
    q_ptr->setSpan(lower, upper);
}

void QxtSpanSliderPrivate::movePressedHandle()
{
    switch (lastPressed)
    {
        case QxtSpanSlider::LowerHandle:
            if (lowerPos != lower)
            {
                bool main = (mainControl == QxtSpanSlider::LowerHandle);
                triggerAction(QAbstractSlider::SliderMove, main);
            }
            break;
        case QxtSpanSlider::UpperHandle:
            if (upperPos != upper)
            {
                bool main = (mainControl == QxtSpanSlider::UpperHandle);
                triggerAction(QAbstractSlider::SliderMove, main);
            }
            break;
        default:
            break;
    }
}

/*!
    \class QxtSpanSlider
    \inmodule QxtWidgets
    \brief QxtSpanSlider 小部件是一个带有两个滑块的 QSlider。
    QxtSpanSlider 是一个带有两个滑块的滑块控件。QxtSpanSlider 适合让用户选择一个介于最小值和最大值之间的范围。
    范围的颜色是根据 QPalette::Highlight 计算的。
    键盘按键的绑定如下表所示：
    \table
    \header \o 方向             \o 按键            \o 滑块
    \row    \o Qt::Horizontal   \o Qt::Key_Left   \o lower
    \row    \o Qt::Horizontal   \o Qt::Key_Right  \o lower
    \row    \o Qt::Horizontal   \o Qt::Key_Up     \o upper
    \row    \o Qt::Horizontal   \o Qt::Key_Down   \o upper
    \row    \o Qt::Vertical     \o Qt::Key_Up     \o lower
    \row    \o Qt::Vertical     \o Qt::Key_Down   \o lower
    \row    \o Qt::Vertical     \o Qt::Key_Left   \o upper
    \row    \o Qt::Vertical     \o Qt::Key_Right  \o upper
    \endtable
    键位绑定是在滑块创建时确定的。在滑块的生命周期内，一个键位始终绑定到相同的滑块。因此，即使滑块的表示从 lower 变为 upper，键位绑定仍然保持不变。
    \image qxtspanslider.png "QxtSpanSlider 在 Plastique 风格下的样式。"
    \bold {注意:} QxtSpanSlider 继承自 QSlider 是由于实现上的原因。调整任何单个滑块特定属性如
    \list
    \o QAbstractSlider::sliderPosition
    \o QAbstractSlider::value
    \endlist
    将不会有任何效果。然而，所有滑块特定属性如
    \list
    \o QAbstractSlider::invertedAppearance
    \o QAbstractSlider::invertedControls
    \o QAbstractSlider::minimum
    \o QAbstractSlider::maximum
    \o QAbstractSlider::orientation
    \o QAbstractSlider::pageStep
    \o QAbstractSlider::singleStep
    \o QSlider::tickInterval
    \o QSlider::tickPosition
    \endlist
    都会被考虑在内。
 */

/*!
    \enum QxtSpanSlider::HandleMovementMode
    此枚举描述了可用的滑块移动模式。
    \value FreeMovement 滑块可以自由移动。
    \value NoCrossing 滑块不能交叉，但仍可以重叠。下限值和上限值可以相同。
    \value NoOverlapping 滑块不能重叠。下限值和上限值不能相同。
 */

/*!
    \enum QxtSpanSlider::SpanHandle
    此枚举描述了可用的范围滑块。
    \omitvalue NoHandle \omit 仅用于内部（暂时）。 \endomit
    \value LowerHandle 下限滑块。
    \value UpperHandle 上限滑块。
 */

/*!
    \fn QxtSpanSlider::lowerValueChanged(int lower)
    每当 \a lower 值发生变化时，都会发出此信号。
 */

/*!
    \fn QxtSpanSlider::upperValueChanged(int upper)
    每当 \a upper 值发生变化时，都会发出此信号。
 */

/*!
    \fn QxtSpanSlider::spanChanged(int lower, int upper)
    每当 \a lower 和 \a upper 值都发生变化时（即范围发生变化时），都会发出此信号。
 */

/*!
    \fn QxtSpanSlider::lowerPositionChanged(int lower)
    每当 \a lower 位置发生变化时，都会发出此信号。
 */

/*!
    \fn QxtSpanSlider::upperPositionChanged(int upper)
    每当 \a upper 位置发生变化时，都会发出此信号。
 */

/*!
    \fn QxtSpanSlider::sliderPressed(SpanHandle handle)
    每当按下 \a handle 时，都会发出此信号。
 */

/*!
    使用 \a parent 构造一个新的 QxtSpanSlider。
 */
QxtSpanSlider::QxtSpanSlider(QWidget* parent) : QSlider(parent), d_ptr(new QxtSpanSliderPrivate())
{
    d_ptr->q_ptr = this;
    connect(this, SIGNAL(rangeChanged(int, int)), d_ptr, SLOT(updateRange(int, int)));
    connect(this, SIGNAL(sliderReleased()), d_ptr, SLOT(movePressedHandle()));
}
/*!
    使用 \a orientation 和 \a parent 构造一个新的 QxtSpanSlider。
 */
QxtSpanSlider::QxtSpanSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent), d_ptr(new QxtSpanSliderPrivate())
{
    d_ptr->q_ptr = this;
    connect(this, SIGNAL(rangeChanged(int, int)), d_ptr, SLOT(updateRange(int, int)));
    connect(this, SIGNAL(sliderReleased()), d_ptr, SLOT(movePressedHandle()));
}

/*!
    销毁 QxtSpanSlider 对象。
 */
QxtSpanSlider::~QxtSpanSlider()
{
}

/*!
    \property QxtSpanSlider::handleMovementMode
    \brief 滑块移动模式
 */
QxtSpanSlider::HandleMovementMode QxtSpanSlider::handleMovementMode() const
{
    return d_ptr->movement;
}

void QxtSpanSlider::setHandleMovementMode(QxtSpanSlider::HandleMovementMode mode)
{
    d_ptr->movement = mode;
}

/*!
    \property QxtSpanSlider::lowerValue
    \brief 范围的下限值
 */
int QxtSpanSlider::lowerValue() const
{
    return qMin(d_ptr->lower, d_ptr->upper);
}

void QxtSpanSlider::setLowerValue(int lower)
{
    setSpan(lower, d_ptr->upper);
}

/*!
    \property QxtSpanSlider::upperValue
    \brief 范围的上限值
 */
int QxtSpanSlider::upperValue() const
{
    return qMax(d_ptr->lower, d_ptr->upper);
}

void QxtSpanSlider::setUpperValue(int upper)
{
    setSpan(d_ptr->lower, upper);
}

/*!
    设置范围，从 \a lower 到 \a upper。
 */
void QxtSpanSlider::setSpan(int lower, int upper)
{
    const int low = qBound(minimum(), qMin(lower, upper), maximum());
    const int upp = qBound(minimum(), qMax(lower, upper), maximum());
    if (low != d_ptr->lower || upp != d_ptr->upper)
    {
        if (low != d_ptr->lower)
        {
            d_ptr->lower = low;
            d_ptr->lowerPos = low;
            emit lowerValueChanged(low);
        }
        if (upp != d_ptr->upper)
        {
            d_ptr->upper = upp;
            d_ptr->upperPos = upp;
            emit upperValueChanged(upp);
        }
        emit spanChanged(d_ptr->lower, d_ptr->upper);
        update();
    }
}

/*!
    \property QxtSpanSlider::lowerPosition
    \brief 范围的下限位置
 */
int QxtSpanSlider::lowerPosition() const
{
    return d_ptr->lowerPos;
}

void QxtSpanSlider::setLowerPosition(int lower)
{
    if (d_ptr->lowerPos != lower)
    {
        d_ptr->lowerPos = lower;
        if (!hasTracking())
            update();
        if (isSliderDown())
            emit lowerPositionChanged(lower);
        if (hasTracking() && !d_ptr->blockTracking)
        {
            bool main = (d_ptr->mainControl == QxtSpanSlider::LowerHandle);
            d_ptr->triggerAction(SliderMove, main);
        }
    }
}

/*!
    \property QxtSpanSlider::upperPosition
    \brief 范围的上限位置
 */

/*!
    获取滑块范围的上限位置。
    \return 返回上限位置的整数值。
 */
int QxtSpanSlider::upperPosition() const
{
    return d_ptr->upperPos;
}

/*!
    设置滑块范围的上限位置。
    \param upper 需要设置的上限位置的整数值。

    如果当前上限位置与提供的值不同，该函数会更新上限位置，并触发相关信号。
    - 如果滑块没有启用跟踪（tracking），在位置变化后会调用 update() 更新滑块的显示。
    - 如果滑块被按下，会触发 \c upperPositionChanged() 信号，通知上限位置发生了变化。
    - 如果滑块启用了跟踪，并且 \c blockTracking 为 false，则会根据当前滑块状态触发适当的滑块动作。

    \note 如果在自由移动（FreeMovement）模式下，新的上限位置比下限位置还要低，滑块的控制会被交换，并更新上限位置。
 */
void QxtSpanSlider::setUpperPosition(int upper)
{
    if (d_ptr->upperPos != upper)
    {
        d_ptr->upperPos = upper;
        if (!hasTracking())
            update();
        if (isSliderDown())
            emit upperPositionChanged(upper);
        if (hasTracking() && !d_ptr->blockTracking)
        {
            bool main = (d_ptr->mainControl == QxtSpanSlider::UpperHandle);
            d_ptr->triggerAction(SliderMove, main);
        }
    }
}

/*!
    \reimp
    处理键盘按键事件，用于改变滑块的位置。
    根据不同的键盘按键，执行相应的滑块动作，例如单步移动或跳转到最小/最大值。

    \param event 指向键盘事件对象的指针。
 */
void QxtSpanSlider::keyPressEvent(QKeyEvent* event)
{
    QSlider::keyPressEvent(event);

    bool main = true;
    SliderAction action = SliderNoAction;
    switch (event->key())
    {
    case Qt::Key_Left:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Right:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Up:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Down:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Home:
        main   = (d_ptr->mainControl == QxtSpanSlider::LowerHandle);
        action = SliderToMinimum;
        break;
    case Qt::Key_End:
        main   = (d_ptr->mainControl == QxtSpanSlider::UpperHandle);
        action = SliderToMaximum;
        break;
    default:
        event->ignore();
        break;
    }

    if (action)
        d_ptr->triggerAction(action, main);
}

/*!
    \reimp
    处理鼠标按下事件。

    此方法用于处理滑块的鼠标按下操作，并确定用户按下的是哪个滑块（上限或下限滑块）。
    如果两个滑块之间的距离等于最小值或最大值，或者鼠标按键未正确释放，则忽略该事件。

    \param event 指向鼠标事件对象的指针。
 */
void QxtSpanSlider::mousePressEvent(QMouseEvent* event)
{
    if (minimum() == maximum() || (event->buttons() ^ event->button()))
    {
        event->ignore();
        return;
    }

    d_ptr->handleMousePress(event->pos(), d_ptr->upperPressed, d_ptr->upper, QxtSpanSlider::UpperHandle);
    if (d_ptr->upperPressed != QStyle::SC_SliderHandle)
        d_ptr->handleMousePress(event->pos(), d_ptr->lowerPressed, d_ptr->lower, QxtSpanSlider::LowerHandle);

    d_ptr->firstMovement = true;
    event->accept();
}

/*!
    \reimp
    处理鼠标移动事件。

    在滑块被按住且移动时，该方法会根据鼠标的位置更新滑块的位置。
    如果移动距离超过指定范围，滑块将不会被更新。
    在自由移动模式下，如果新的滑块位置跨越了另一滑块的位置，两个滑块的控制会被交换。

    \param event 指向鼠标事件对象的指针。
 */
void QxtSpanSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (d_ptr->lowerPressed != QStyle::SC_SliderHandle && d_ptr->upperPressed != QStyle::SC_SliderHandle)
    {
        event->ignore();
        return;
    }

    QStyleOptionSlider opt;
    d_ptr->initStyleOption(&opt);
    const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
    int newPosition = d_ptr->pixelPosToRangeValue(d_ptr->pick(event->pos()) - d_ptr->offset);
    if (m >= 0)
    {
        const QRect r = rect().adjusted(-m, -m, m, m);
        if (!r.contains(event->pos()))
        {
            newPosition = d_ptr->position;
        }
    }

    // 在第一次移动时，选择优先操作的滑块
    if (d_ptr->firstMovement)
    {
        if (d_ptr->lower == d_ptr->upper)
        {
            if (newPosition < lowerValue())
            {
                d_ptr->swapControls();
                d_ptr->firstMovement = false;
            }
        }
        else
        {
            d_ptr->firstMovement = false;
        }
    }

    if (d_ptr->lowerPressed == QStyle::SC_SliderHandle)
    {
        if (d_ptr->movement == NoCrossing)
            newPosition = qMin(newPosition, upperValue());
        else if (d_ptr->movement == NoOverlapping)
            newPosition = qMin(newPosition, upperValue() - 1);

        if (d_ptr->movement == FreeMovement && newPosition > d_ptr->upper)
        {
            d_ptr->swapControls();
            setUpperPosition(newPosition);
        }
        else
        {
            setLowerPosition(newPosition);
        }
    }
    else if (d_ptr->upperPressed == QStyle::SC_SliderHandle)
    {
        if (d_ptr->movement == NoCrossing)
            newPosition = qMax(newPosition, lowerValue());
        else if (d_ptr->movement == NoOverlapping)
            newPosition = qMax(newPosition, lowerValue() + 1);

        if (d_ptr->movement == FreeMovement && newPosition < d_ptr->lower)
        {
            d_ptr->swapControls();
            setLowerPosition(newPosition);
        }
        else
        {
            setUpperPosition(newPosition);
        }
    }
    event->accept();
}

/*!
    \reimp
    该函数重写了 QSlider 的 mouseReleaseEvent，用于处理鼠标释放事件。
    当鼠标按钮被释放时，将会调用该函数。
 */
void QxtSpanSlider::mouseReleaseEvent(QMouseEvent* event)
{
    // 调用父类的 mouseReleaseEvent 函数，确保基本的释放事件处理得以执行
    QSlider::mouseReleaseEvent(event);

    // 将滑块设置为未按下状态
    setSliderDown(false);

    // 重置下限和上限滑块的按压状态
    d_ptr->lowerPressed = QStyle::SC_None;
    d_ptr->upperPressed = QStyle::SC_None;

    // 更新组件的外观
    update();
}

/*!
    \reimp
    该函数重写了 QSlider 的 paintEvent，用于绘制滑块组件的自定义外观。
    当需要重新绘制滑块组件时，将会调用该函数。
 */
 /*
void QxtSpanSlider::paintEvent(QPaintEvent* event)
{
    // Q_UNUSED 用于标识未使用的参数以避免编译器警告
    Q_UNUSED(event);

    // 创建 QStylePainter 对象，用于绘制组件
    QStylePainter painter(this);

    // 创建 QStyleOptionSlider 对象，并初始化选项
    QStyleOptionSlider opt;
    d_ptr->initStyleOption(&opt);

    // 绘制滑槽和刻度标记
    opt.sliderValue = 0;
    opt.sliderPosition = 0;
    opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // 计算下限滑块的矩形区域
    opt.sliderPosition = d_ptr->lowerPos;
    const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int lrv  = d_ptr->pick(lr.center());

    // 计算上限滑块的矩形区域
    opt.sliderPosition = d_ptr->upperPos;
    const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int urv  = d_ptr->pick(ur.center());

    // 计算 span（滑块之间的范围）的矩形区域
    const int minv = qMin(lrv, urv);
    const int maxv = qMax(lrv, urv);
    const QPoint c = QRect(lr.center(), ur.center()).center();
    QRect spanRect;
    if (orientation() == Qt::Horizontal)
        spanRect = QRect(QPoint(minv, c.y() - 2), QPoint(maxv, c.y() + 1));
    else
        spanRect = QRect(QPoint(c.x() - 2, minv), QPoint(c.x() + 1, maxv));

    // 绘制 span 的外观
    d_ptr->drawSpan(&painter, spanRect);

    // 根据最后一个被按下的滑块，绘制滑块的外观
    switch (d_ptr->lastPressed)
    {
    case QxtSpanSlider::LowerHandle:
        // 优先绘制上限滑块，然后绘制下限滑块
        d_ptr->drawHandle(&painter, QxtSpanSlider::UpperHandle);
        d_ptr->drawHandle(&painter, QxtSpanSlider::LowerHandle);
        break;
    case QxtSpanSlider::UpperHandle:
    default:
        // 优先绘制下限滑块，然后绘制上限滑块
        d_ptr->drawHandle(&painter, QxtSpanSlider::LowerHandle);
        d_ptr->drawHandle(&painter, QxtSpanSlider::UpperHandle);
        break;
    }
}
*/

void QxtSpanSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const
{
    // 从 style 获取 QSS 背景色
    const QStyleOptionSlider opt;
    QColor spanColor = q_ptr->palette().color(QPalette::Highlight);

    QBrush brush = q_ptr->palette().brush(QPalette::Highlight);
    painter->setBrush(brush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);
}

// QxtSpanSliderPrivate.cpp

void QxtSpanSliderPrivate::drawHandle(QStylePainter* painter, QxtSpanSlider::SpanHandle handle) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);

    // 获取 style 的 handle 矩形
    QRect handleRect = q_ptr->style()->subControlRect(QStyle::CC_Slider, &opt,
                                                      QStyle::SC_SliderHandle, q_ptr);

    painter->save();
    QColor color = (handle == QxtSpanSlider::LowerHandle) ? QColor("#009688") : QColor("#33b5aa");
    painter->setBrush(color);
    painter->setPen(QPen(Qt::black));
    painter->drawEllipse(handleRect);
    painter->restore();
}

void QxtSpanSlider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionSlider opt;
    d_ptr->initStyleOption(&opt);

    // 1. 绘制 groove
    opt.subControls = QStyle::SC_SliderGroove;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // 2. 计算 handle 矩形
    opt.sliderPosition = d_ptr->lowerPos;
    QRect lowerRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    opt.sliderPosition = d_ptr->upperPos;
    QRect upperRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    // 3. 绘制 span（两个 handle 之间）
    QRect spanRect;
    if (orientation() == Qt::Horizontal)
    {
        int x1 = qMin(lowerRect.center().x(), upperRect.center().x());
        int x2 = qMax(lowerRect.center().x(), upperRect.center().x());
        spanRect = QRect(QPoint(x1, height()/2 - 3), QPoint(x2, height()/2 + 3));
    }
    else
    {
        int y1 = qMin(lowerRect.center().y(), upperRect.center().y());
        int y2 = qMax(lowerRect.center().y(), upperRect.center().y());
        spanRect = QRect(QPoint(width()/2 - 3, y1), QPoint(width()/2 + 3, y2));
    }

    painter.save();
    painter.setBrush(palette().color(QPalette::Highlight));
    painter.setPen(Qt::NoPen);
    painter.drawRect(spanRect);
    painter.restore();

    // 4. 绘制 handle
    auto drawHandle = [&](const QRect& r, const QColor& color){
        painter.save();
        painter.setBrush(color);
        painter.setPen(Qt::black);
        painter.drawEllipse(r);
        painter.restore();
    };

    drawHandle(lowerRect, QColor("#009688"));
    drawHandle(upperRect, QColor("#33b5aa"));
}

