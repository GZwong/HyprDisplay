#pragma once

#include <memory>
#include <string>

class LogService {
public:
    virtual ~LogService() = default;

    virtual void logInfo(const std::string &message) = 0;
    virtual void logError(const std::string &message) = 0;

    // Call exactly once at startup (in main) before any logging.
    static void init(std::unique_ptr<LogService> impl);

    // Returns the active logger. Aborts if init() was never called.
    static LogService &instance();

private:
    static std::unique_ptr<LogService> s_instance;
};
