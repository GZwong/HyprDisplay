#include "services/FileLogService.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

FileLogService::FileLogService(const std::string &filePath)
    : m_file(filePath, std::ios::app)
{}

void FileLogService::logInfo(const std::string &message)
{
    write("INFO ", message);
}

void FileLogService::logError(const std::string &message)
{
    write("ERROR", message);
}

void FileLogService::write(const std::string &level, const std::string &message)
{
    if (!m_file.is_open())
        return;

    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
        << "  [" << level << "]  " << message << '\n';

    m_file << oss.str();
    m_file.flush();
}
