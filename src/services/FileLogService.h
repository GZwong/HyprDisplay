#pragma once

#include <fstream>
#include "services/LogService.h"

class FileLogService : public LogService {
public:
    explicit FileLogService(const std::string &filePath);

    void logInfo(const std::string &message) override;
    void logError(const std::string &message) override;

private:
    void write(const std::string &level, const std::string &message);

    std::ofstream m_file;
};
