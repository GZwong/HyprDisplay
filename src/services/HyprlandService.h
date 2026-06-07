#pragma once

#include <QList>
#include <QString>
#include "models/Monitor.h"

// Communicates with Hyprland via the hyprctl CLI tool.
// All I/O is synchronous and blocking — call from a thread or accept the latency
// (typical hyprctl round-trip is <50 ms).
class HyprlandService {
public:
    // Returns all monitors Hyprland knows about (enabled and disabled).
    QList<Monitor> fetchMonitors();

    // Applies the full monitor list to Hyprland.
    // Returns true only if every keyword command returned "ok".
    bool applyConfig(const QList<Monitor> &monitors);

private:
    QString runHyprctl(const QStringList &args);
};
