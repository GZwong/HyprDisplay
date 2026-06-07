#pragma once

#include <QString>

struct Monitor {
    int id = -1;
    QString name;
    QString description;
    int width = 0;
    int height = 0;
    double refreshRate = 60.0;
    int x = 0;
    int y = 0;
    double scale = 1.0;
    bool enabled = true;
};
