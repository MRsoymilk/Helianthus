#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QGraphicsSimpleTextItem>
#include <QtCharts>
#include <limits>

class MyChartView : public QChartView
{
    Q_OBJECT

public:
    MyChartView(QChart *chart, QWidget *parent = nullptr)
        : QChartView(chart, parent)
    {
        setMouseTracking(true);
        setChart(chart);
    }

    void setChart(QChart *chart)
    {
        QChartView::setChart(chart);
        m_chart = chart;

        clearHelpers();
        initHelpers();
        applyCustomStyle(); // 美化样式
    }

private:
    void initHelpers()
    {
        if (!m_chart)
            return;

        QPen crossPen(QColor(0, 170, 255));
        crossPen.setStyle(Qt::DotLine);
        crossPen.setWidth(1);

        m_lineV = m_chart->scene()->addLine(QLineF(), crossPen);
        m_lineH = m_chart->scene()->addLine(QLineF(), crossPen);

        m_coordText = new QGraphicsSimpleTextItem(m_chart);
        QFont font = m_coordText->font();
        font.setPointSize(10);
        font.setBold(true);
        m_coordText->setFont(font);
        m_coordText->setBrush(QBrush(QColor("#333333")));
        m_coordText->setZValue(11);

        m_marker = m_chart->scene()->addEllipse(QRectF(0, 0, 10, 10),
                                                QPen(QColor("#ff9500"), 2),
                                                QBrush(QColor("#ffd180")));
        m_marker->setZValue(10);
        m_marker->hide();
    }

    void clearHelpers()
    {
        if (m_lineV) {
            scene()->removeItem(m_lineV);
            delete m_lineV;
            m_lineV = nullptr;
        }
        if (m_lineH) {
            scene()->removeItem(m_lineH);
            delete m_lineH;
            m_lineH = nullptr;
        }
        if (m_coordText) {
            delete m_coordText;
            m_coordText = nullptr;
        }
        if (m_marker) {
            scene()->removeItem(m_marker);
            delete m_marker;
            m_marker = nullptr;
        }
    }

    void applyCustomStyle()
    {
        if (!m_chart)
            return;

        // Background gradient (pure white to light gray)
        QLinearGradient bgGrad;
        bgGrad.setStart(QPointF(0, 0));
        bgGrad.setFinalStop(QPointF(0, 1));
        bgGrad.setColorAt(0.0, QColor("#ffffff")); // Pure white
        bgGrad.setColorAt(1.0, QColor("#f5f5f5")); // Light gray
        m_chart->setBackgroundBrush(QBrush(bgGrad));

        // Set chart theme
        m_chart->setTheme(QChart::ChartThemeLight);

        // Customize axes
        for (auto *axis : m_chart->axes()) {
            axis->setGridLineColor(QColor("#e0e0e0"));      // Light gray grid lines
            axis->setLabelsColor(QColor("#333333"));        // Dark gray labels
            axis->setLinePen(QPen(QColor("#4a90e2"), 1));   // Soft blue axis lines
            axis->setTitleBrush(QBrush(QColor("#333333"))); // Dark gray title
        }

        // Customize plot area background
        m_chart->setPlotAreaBackgroundBrush(QBrush(QColor("#fafafa"))); // Off-white plot area
        m_chart->setPlotAreaBackgroundVisible(true);
    }

    void applyDarkCustomStyle()
    {
        if (!m_chart)
            return;

        // Background gradient (dark purple to black)
        QLinearGradient bgGrad;
        bgGrad.setStart(QPointF(0, 0));
        bgGrad.setFinalStop(QPointF(0, 1));
        bgGrad.setColorAt(0.0, QColor("#282a36")); // Dracula background (dark purple)
        bgGrad.setColorAt(1.0, QColor("#1e1f29")); // Darker shade
        m_chart->setBackgroundBrush(QBrush(bgGrad));

        // Set chart theme (optional, using custom styling instead of predefined theme)
        m_chart->setTheme(QChart::ChartThemeDark);

        // Customize axes
        for (auto *axis : m_chart->axes()) {
            axis->setGridLineColor(QColor("#44475a"));      // Dracula foreground (muted gray)
            axis->setLabelsColor(QColor("#f8f8f2"));        // Dracula text (light gray/white)
            axis->setLinePen(QPen(QColor("#6272a4"), 1));   // Dracula comment color for axis lines
            axis->setTitleBrush(QBrush(QColor("#f8f8f2"))); // Title text color
        }

        // Optional: Customize plot area background
        m_chart->setPlotAreaBackgroundBrush(QBrush(QColor("#282a36")));
        m_chart->setPlotAreaBackgroundVisible(true);
    }

protected:
    void mouseMoveEvent(QMouseEvent *event) override
    {
        QPointF pos = event->pos();
        QPointF value = m_chart->mapToValue(pos);

        QRectF plotArea = m_chart->plotArea();
        if (!plotArea.contains(pos)) {
            m_lineV->hide();
            m_lineH->hide();
            m_coordText->hide();
            m_marker->hide();
            return;
        }

        m_lineV->show();
        m_lineH->show();
        m_coordText->show();

        qreal x = value.x();
        qreal y = value.y();

        // 查找最近点
        QPointF closestPoint;
        qreal minDist = std::numeric_limits<qreal>::max();
        for (auto *series : m_chart->series()) {
            auto *xySeries = qobject_cast<QXYSeries *>(series);
            if (!xySeries)
                continue;

            const auto &points = xySeries->points();
            for (const QPointF &p : points) {
                qreal dist = std::hypot(p.x() - x, p.y() - y);
                if (dist < minDist) {
                    minDist = dist;
                    closestPoint = p;
                }
            }
        }

        // 更新十字线
        QPointF top = QPointF(pos.x(), plotArea.top());
        QPointF bottom = QPointF(pos.x(), plotArea.bottom());
        QPointF left = QPointF(plotArea.left(), pos.y());
        QPointF right = QPointF(plotArea.right(), pos.y());

        m_lineV->setLine(QLineF(mapToScene(top.toPoint()), mapToScene(bottom.toPoint())));
        m_lineH->setLine(QLineF(mapToScene(left.toPoint()), mapToScene(right.toPoint())));

        // 更新吸附点
        QPointF closestScenePos = mapToScene(m_chart->mapToPosition(closestPoint).toPoint());
        m_marker->setRect(closestScenePos.x() - 5, closestScenePos.y() - 5, 10, 10);
        m_marker->show();

        // 坐标文本
        QString coordText = QString("X: %1\nY: %2")
                                .arg(closestPoint.x(), 0, 'f', 3)
                                .arg(closestPoint.y(), 0, 'f', 3);
        m_coordText->setText(coordText);
        m_coordText->setPos(closestScenePos + QPointF(10, -30));

        QChartView::mouseMoveEvent(event);
    }

private:
    QChart *m_chart = nullptr;
    QGraphicsLineItem *m_lineV = nullptr;
    QGraphicsLineItem *m_lineH = nullptr;
    QGraphicsSimpleTextItem *m_coordText = nullptr;
    QGraphicsEllipseItem *m_marker = nullptr;
};

#endif // MYCHARTVIEW_H
