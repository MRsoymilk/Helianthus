#include "flowpanel.h"
#include <QGraphicsDropShadowEffect>
#include <QToolButton>
#include "ui_flowpanel.h"

FlowPanel::FlowPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FlowPanel)
{
    ui->setupUi(this);
    init();
}

FlowPanel::~FlowPanel()
{
    delete ui;
}

void FlowPanel::init()
{
    this->setObjectName("FlowPanel");
    setAttribute(Qt::WA_StyledBackground, true); // 允许样式生效

    setStyleSheet(R"(
    QWidget#FlowPanel {
        background-color: white;
        border-radius: 8px;
        border: 1px solid #ccc;
    }
)");

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15);
    shadow->setOffset(0, 0);
    shadow->setColor(QColor(0, 0, 0, 80));
    this->setGraphicsEffect(shadow);

    if (parentWidget()) {
        move(parentWidget()->width() - m_visual_size, 0);
    }

    animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    ui->labelFlowArrow->setFixedSize(24, 24);
    QPixmap pix(":/res/icons/left.png");
    ui->labelFlowArrow->setPixmap(pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelFlowArrow->setScaledContents(false);
}

void FlowPanel::enterEvent(QEnterEvent *event)
{
    if (m_pinned) {
        return;
    }
    Q_UNUSED(event);
    if (!parentWidget())
        return;

    ui->labelFlowArrow->setFixedSize(24, 24);
    QPixmap pix(":/res/icons/right.png");
    ui->labelFlowArrow->setPixmap(pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelFlowArrow->setScaledContents(false);

    animation->stop();
    animation->setStartValue(pos());
    animation->setEndValue(QPoint(parentWidget()->width() - width(), 0));
    animation->start();
}

void FlowPanel::leaveEvent(QEvent *event)
{
    if (m_pinned) {
        return;
    }
    Q_UNUSED(event);
    if (!parentWidget()) {
        return;
    }

    ui->labelFlowArrow->setFixedSize(24, 24);
    QPixmap pix(":/res/icons/left.png");
    ui->labelFlowArrow->setPixmap(pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelFlowArrow->setScaledContents(false);

    animation->stop();
    animation->setStartValue(pos());
    animation->setEndValue(QPoint(parentWidget()->width() - m_visual_size, 0));
    animation->start();
}

void FlowPanel::FlowPanel::updatePosition()
{
    if (m_pinned) {
        return;
    }
    if (!parentWidget()) {
        return;
    }

    setFixedHeight(parentWidget()->height());

    int x = pos().x() > (parentWidget()->width() - width() - 10)
                ? parentWidget()->width() - m_visual_size
                : parentWidget()->width() - width();
    move(x, 0);
}

void FlowPanel::on_btnSerial_clicked()
{
    emit switchForm(FORM::SERIAL);
}

void FlowPanel::on_btnResult_clicked()
{
    emit switchForm(FORM::RESULT);
}

void FlowPanel::on_btnPlot_clicked()
{
    emit switchForm(FORM::PLOT);
}

void FlowPanel::on_btnHistory_clicked()
{
    emit switchForm(FORM::HISTORY);
}

void FlowPanel::on_btnSetting_clicked()
{
    emit switchForm(FORM::SETTING);
}

void FlowPanel::on_tBtnPin_clicked()
{
    m_pinned = !m_pinned;

    emit pinPanel(m_pinned);

    if (m_pinned) {
        // ui->tBtnPin->setIcon(QIcon(":/res/icons/pin_on.png"));
        animation->stop();
    } else {
        // ui->tBtnPin->setIcon(QIcon(":/res/icons/pin_off.png"));
        animation->stop();
        animation->setStartValue(pos());
        animation->setEndValue(QPoint(parentWidget()->width() - m_visual_size, 0));
        animation->start();
    }
}
