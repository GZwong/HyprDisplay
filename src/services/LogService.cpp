#include "services/LogService.h"

#include <stdexcept>

// Static member definition — lives in exactly one translation unit.
std::unique_ptr<LogService> LogService::s_instance;

void LogService::init(std::unique_ptr<LogService> impl)
{
    s_instance = std::move(impl);
}

LogService &LogService::instance()
{
    if (!s_instance)
        throw std::runtime_error("LogService::init() must be called before instance()");
    return *s_instance;
}
