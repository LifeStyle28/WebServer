#pragma once

#include "use_cases.h"

namespace app
{

class Application
{
public:
    Application(model::Config& config, const std::filesystem::path& scriptPath,
        const std::filesystem::path& resultPath, const std::filesystem::path& webPath,
        const std::filesystem::path& configJsonPath);
    CreateConnectionResult CreateConnection(const model::Contract::Id id,
        const size_t duration) const;
    std::string GetResultFileName(const std::string& body, const Token& token) const;
    void Tick(const std::chrono::steady_clock::time_point& timeNow);
    void ChangePercent(const size_t percent);
private:
    model::Config& m_config;
    ConnectionTokens m_tokens;
    CreateConnectionUseCase m_createConnection{m_config, m_tokens};
    CreateResultFileUseCase m_createResultFile;
    TimerUseCase m_timer{m_tokens};
    ChangePercentUseCase m_changePercent;
};

} // namespace app
