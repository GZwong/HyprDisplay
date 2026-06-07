#pragma once

#include <QWidget>
#include <QList>
#include <QColor>
#include "models/Monitor.h"

// Custom widget that renders monitors as draggable coloured rectangles.
// Coordinates: all internal positions stay in "monitor space" (real pixel values).
// The scale factor converts monitor space → widget (screen) space for painting.
class DisplayArrangeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayArrangeWidget(QWidget *parent = nullptr);

    void setMonitors(const QList<Monitor> &monitors);

signals:
    // Emitted with monitor-space coordinates after the user drops a monitor.
    void monitorMoved(int id, int x, int y);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct MonitorItem {
        Monitor monitor;
        QRect   widgetRect;  // position/size in widget (screen) pixels
        QColor  color;
    };

    void recomputeLayout();
    QRect toWidgetRect(const Monitor &m) const;

    // Snaps m's edges to nearby monitors within SNAP_SCREEN_PX screen pixels.
    void snapToNeighbours(Monitor &m) const;

    int itemIndexAt(const QPoint &widgetPos) const;

    QList<MonitorItem> m_items;

    double m_scale = 0.1;    // monitor-space px → widget px
    QPoint m_canvasOffset;   // top-left of monitor (0,0) in widget coords

    int    m_dragIndex         = -1;
    QPoint m_dragStartWidget;         // widget pos where the drag began
    QPoint m_dragStartMonitorOrigin;  // monitor.x/y when the drag began

    static constexpr int SNAP_SCREEN_PX = 18;
    static const QList<QColor> k_colors;
};
