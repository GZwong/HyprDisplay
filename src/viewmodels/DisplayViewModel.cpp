#include "viewmodels/DisplayViewModel.h"

DisplayViewModel::DisplayViewModel(QObject *parent)
    : QObject(parent)
{}

const QList<Monitor> &DisplayViewModel::monitors() const
{
    return m_monitors;
}

void DisplayViewModel::refresh()
{
    m_monitors = m_service.fetchMonitors();
    emit monitorsChanged();
}

void DisplayViewModel::applyChanges()
{
    if (m_service.applyConfig(m_monitors)) {
        emit applySucceeded();
        refresh(); // re-sync with what Hyprland actually set
    } else {
        emit applyFailed("Failed to apply configuration. Is Hyprland running?");
    }
}

void DisplayViewModel::setMonitorEnabled(int id, bool enabled)
{
    if (!enabled && !canDisableMonitor(id)) {
        emit errorOccurred("Cannot disable the last active monitor.");
        emit monitorsChanged(); // re-sync view so the checkbox reverts
        return;
    }

    for (Monitor &m : m_monitors) {
        if (m.id == id) {
            m.enabled = enabled;
            break;
        }
    }
    emit monitorsChanged();
}

void DisplayViewModel::setMonitorPosition(int id, int x, int y)
{
    for (Monitor &m : m_monitors) {
        if (m.id == id) {
            m.x = x;
            m.y = y;
            emit monitorsChanged();
            return;
        }
    }
}

bool DisplayViewModel::canDisableMonitor(int id) const
{
    int activeOthers = 0;
    for (const Monitor &m : m_monitors) {
        if (m.enabled && m.id != id)
            ++activeOthers;
    }
    return activeOthers > 0;
}

int DisplayViewModel::activeMonitorCount() const
{
    int count = 0;
    for (const Monitor &m : m_monitors)
        if (m.enabled)
            ++count;
    return count;
}
