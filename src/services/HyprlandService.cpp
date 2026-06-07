#include "services/HyprlandService.h"
#include "services/LogService.h"

#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QString HyprlandService::runHyprctl(const QStringList &args)
{
    const std::string cmd = ("hyprctl " + args.join(' ')).toStdString();
    LogService::instance().logInfo("Running: " + cmd);

    QProcess proc;
    proc.start("hyprctl", args);
    if (!proc.waitForFinished(5000)) {
        LogService::instance().logError("Timed out: " + cmd);
        return {};
    }

    const QString output = QString::fromUtf8(proc.readAllStandardOutput());
    const QString errOutput = QString::fromUtf8(proc.readAllStandardError());

    if (!output.isEmpty())
        LogService::instance().logInfo("Output: " + output.trimmed().toStdString());
    if (!errOutput.isEmpty())
        LogService::instance().logError("Stderr: " + errOutput.trimmed().toStdString());

    return output;
}

QList<Monitor> HyprlandService::fetchMonitors()
{
    const QString output = runHyprctl({"monitors", "all", "-j"});
    const QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    if (!doc.isArray()) {
        LogService::instance().logError("Failed to parse monitor JSON");
        return {};
    }

    QList<Monitor> monitors;
    for (const QJsonValue &val : doc.array()) {
        const QJsonObject obj = val.toObject();
        Monitor m;
        m.id          = obj["id"].toInt();
        m.name        = obj["name"].toString();
        m.description = obj["description"].toString();
        m.width       = obj["width"].toInt();
        m.height      = obj["height"].toInt();
        m.refreshRate = obj["refreshRate"].toDouble();
        m.x           = obj["x"].toInt();
        m.y           = obj["y"].toInt();
        m.scale       = obj["scale"].toDouble(1.0);
        m.enabled     = !obj["disabled"].toBool(false);
        monitors.append(m);
    }
    return monitors;
}

bool HyprlandService::applyConfig(const QList<Monitor> &monitors)
{
    for (const Monitor &m : monitors) {
        QString expr;

        if (!m.enabled) {
            expr = QString(R"(hl.monitor({ output = "%1", disabled = true }))")
                       .arg(m.name);
        } else {
            const QString mode = QString("%1x%2@%3")
                                     .arg(m.width)
                                     .arg(m.height)
                                     .arg(m.refreshRate, 0, 'f', 3);
            const QString position = QString("%1x%2").arg(m.x).arg(m.y);
            expr = QString(R"(hl.monitor({ output = "%1", mode = "%2", position = "%3", scale = "%4", disabled = false }))")
                       .arg(m.name)
                       .arg(mode)
                       .arg(position)
                       .arg(m.scale, 0, 'f', 2);
        }

        const QString result = runHyprctl({"eval", expr});
        if (!result.trimmed().startsWith("ok")) {
            LogService::instance().logError(
                "Command failed for monitor: " + m.name.toStdString());
            return false;
        }
    }
    return true;
}
