#pragma once

#include <QObject>
#include <QList>
#include "models/Monitor.h"
#include "services/HyprlandService.h"

// The ViewModel is the bridge between the View and the data layer.
// It owns the authoritative in-memory monitor list, validates state transitions,
// and notifies the View through signals — the View never touches HyprlandService directly.
class DisplayViewModel : public QObject
{
    Q_OBJECT

public:
    explicit DisplayViewModel(QObject *parent = nullptr);

    const QList<Monitor> &monitors() const;

    // Returns false and emits errorOccurred if the monitor is the last active one.
    void setMonitorEnabled(int id, bool enabled);

    // Records a position change from the drag canvas.
    void setMonitorPosition(int id, int x, int y);

    bool canDisableMonitor(int id) const;
    int activeMonitorCount() const;

public slots:
    void refresh();
    void applyChanges();

signals:
    void monitorsChanged();
    void errorOccurred(const QString &message);
    void applySucceeded();
    void applyFailed(const QString &message);

private:
    QList<Monitor> m_monitors;
    HyprlandService m_service;
};
