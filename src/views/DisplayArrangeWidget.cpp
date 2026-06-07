#include "views/DisplayArrangeWidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <climits>
#include <cmath>

// Hyprland positions (x, y) are in logical pixels, but width/height are physical
// buffer pixels. Divide by scale to get the logical size used for layout.
static int logicalW(const Monitor &m) { return static_cast<int>(std::round(m.width  / m.scale)); }
static int logicalH(const Monitor &m) { return static_cast<int>(std::round(m.height / m.scale)); }

const QList<QColor> DisplayArrangeWidget::k_colors = {
    QColor(0x42, 0xA5, 0xF5), // blue
    QColor(0x66, 0xBB, 0x6A), // green
    QColor(0xFF, 0xA7, 0x26), // orange
    QColor(0xAB, 0x47, 0xBC), // purple
    QColor(0xEF, 0x53, 0x50), // red
};

DisplayArrangeWidget::DisplayArrangeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void DisplayArrangeWidget::setMonitors(const QList<Monitor> &monitors)
{
    m_items.clear();
    for (int i = 0; i < monitors.size(); ++i) {
        MonitorItem item;
        item.monitor = monitors[i];
        item.color   = k_colors[i % k_colors.size()];
        m_items.append(item);
    }
    recomputeLayout();
    update();
}

// ---------------------------------------------------------------------------
// Layout: compute scale and canvas offset so all monitors fit with padding
// ---------------------------------------------------------------------------

void DisplayArrangeWidget::recomputeLayout()
{
    if (m_items.isEmpty())
        return;

    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
    for (const MonitorItem &item : m_items) {
        minX = std::min(minX, item.monitor.x);
        minY = std::min(minY, item.monitor.y);
        maxX = std::max(maxX, item.monitor.x + logicalW(item.monitor));
        maxY = std::max(maxY, item.monitor.y + logicalH(item.monitor));
    }

    const int padding   = 32;
    const int totalW    = maxX - minX;
    const int totalH    = maxY - minY;
    const int availW    = width()  - padding * 2;
    const int availH    = height() - padding * 2;

    if (totalW > 0 && totalH > 0) {
        const double scaleX = static_cast<double>(availW) / totalW;
        const double scaleY = static_cast<double>(availH) / totalH;
        m_scale = std::min({scaleX, scaleY, 0.30}); // cap: don't zoom in too much
        m_scale = std::max(m_scale, 0.04);           // floor: don't shrink to nothing
    }

    const int scaledW = static_cast<int>(totalW * m_scale);
    const int scaledH = static_cast<int>(totalH * m_scale);

    // Centre the whole arrangement inside the widget
    m_canvasOffset = QPoint(
        (width()  - scaledW) / 2 - static_cast<int>(minX * m_scale),
        (height() - scaledH) / 2 - static_cast<int>(minY * m_scale)
    );

    for (MonitorItem &item : m_items)
        item.widgetRect = toWidgetRect(item.monitor);
}

QRect DisplayArrangeWidget::toWidgetRect(const Monitor &m) const
{
    return QRect(
        static_cast<int>(m.x * m_scale) + m_canvasOffset.x(),
        static_cast<int>(m.y * m_scale) + m_canvasOffset.y(),
        static_cast<int>(logicalW(m) * m_scale),
        static_cast<int>(logicalH(m) * m_scale)
    );
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

void DisplayArrangeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Canvas background
    p.fillRect(rect(), QColor(0x18, 0x18, 0x28));

    for (int i = 0; i < m_items.size(); ++i) {
        const MonitorItem &item = m_items[i];
        const QRect &r = item.widgetRect;

        const bool dragging = (i == m_dragIndex);
        QColor fill = item.monitor.enabled ? item.color : QColor(70, 70, 80);
        if (dragging)
            fill = fill.lighter(130);

        // Drop shadow
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 70));
        p.drawRoundedRect(r.adjusted(4, 4, 4, 4), 7, 7);

        // Monitor rectangle
        p.setBrush(fill);
        p.setPen(QPen(fill.lighter(160), 1.5));
        p.drawRoundedRect(r, 7, 7);

        // Label
        p.setPen(item.monitor.enabled ? Qt::white : QColor(160, 160, 160));

        QFont nameFont = p.font();
        nameFont.setBold(true);
        nameFont.setPointSize(9);
        p.setFont(nameFont);

        const QString resLine = item.monitor.enabled
            ? QString("%1x%2").arg(item.monitor.width).arg(item.monitor.height)  // physical res
            : QStringLiteral("disabled");

        p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter,
                   item.monitor.name + "\n" + resLine);
    }
}

// ---------------------------------------------------------------------------
// Mouse events — drag and drop
// ---------------------------------------------------------------------------

int DisplayArrangeWidget::itemIndexAt(const QPoint &pos) const
{
    // Iterate in reverse so the topmost-painted item wins
    for (int i = m_items.size() - 1; i >= 0; --i) {
        if (m_items[i].widgetRect.contains(pos))
            return i;
    }
    return -1;
}

void DisplayArrangeWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const int idx = itemIndexAt(event->pos());
    if (idx < 0)
        return;

    m_dragIndex              = idx;
    m_dragStartWidget        = event->pos();
    m_dragStartMonitorOrigin = QPoint(m_items[idx].monitor.x, m_items[idx].monitor.y);
    setCursor(Qt::ClosedHandCursor);
}

void DisplayArrangeWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragIndex < 0) {
        setCursor(itemIndexAt(event->pos()) >= 0 ? Qt::OpenHandCursor : Qt::ArrowCursor);
        return;
    }

    const QPoint delta = event->pos() - m_dragStartWidget;
    MonitorItem &item  = m_items[m_dragIndex];

    item.monitor.x = m_dragStartMonitorOrigin.x() + static_cast<int>(delta.x() / m_scale);
    item.monitor.y = m_dragStartMonitorOrigin.y() + static_cast<int>(delta.y() / m_scale);
    item.widgetRect = toWidgetRect(item.monitor);

    update();
}

void DisplayArrangeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragIndex < 0 || event->button() != Qt::LeftButton)
        return;

    MonitorItem &item = m_items[m_dragIndex];
    snapToNeighbours(item.monitor);
    item.widgetRect = toWidgetRect(item.monitor);

    emit monitorMoved(item.monitor.id, item.monitor.x, item.monitor.y);

    m_dragIndex = -1;
    setCursor(Qt::ArrowCursor);
    recomputeLayout();
    update();
}

void DisplayArrangeWidget::resizeEvent(QResizeEvent *)
{
    recomputeLayout();
}

// ---------------------------------------------------------------------------
// Snap-to-neighbour: pull the dragged monitor's edges toward adjacent ones
// ---------------------------------------------------------------------------

void DisplayArrangeWidget::snapToNeighbours(Monitor &m) const
{
    // Convert screen-pixel snap threshold to logical monitor-space pixels
    const int snapMon = static_cast<int>(SNAP_SCREEN_PX / m_scale);
    const int mW = logicalW(m);
    const int mH = logicalH(m);

    for (const MonitorItem &item : m_items) {
        if (item.monitor.id == m.id)
            continue;

        const Monitor &o  = item.monitor;
        const int     oW  = logicalW(o);
        const int     oH  = logicalH(o);

        // --- Horizontal axis ---
        if (std::abs(m.x - (o.x + oW)) < snapMon)
            m.x = o.x + oW;                  // m's left to o's right
        else if (std::abs((m.x + mW) - o.x) < snapMon)
            m.x = o.x - mW;                  // m's right to o's left
        else if (std::abs(m.x - o.x) < snapMon)
            m.x = o.x;                        // align left edges
        else if (std::abs((m.x + mW) - (o.x + oW)) < snapMon)
            m.x = o.x + oW - mW;             // align right edges

        // --- Vertical axis ---
        if (std::abs(m.y - (o.y + oH)) < snapMon)
            m.y = o.y + oH;                  // m's top to o's bottom
        else if (std::abs((m.y + mH) - o.y) < snapMon)
            m.y = o.y - mH;                  // m's bottom to o's top
        else if (std::abs(m.y - o.y) < snapMon)
            m.y = o.y;                        // align top edges
        else if (std::abs((m.y + mH) - (o.y + oH)) < snapMon)
            m.y = o.y + oH - mH;             // align bottom edges
    }
}
