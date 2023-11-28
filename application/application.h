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
    void ChangeContractParams(const size_t percent, std::string email);
    size_t GetContractPercent() const noexcept;
    std::string_view GetContractEmail() const noexcept;
private:
    model::Config& m_config;
    ConnectionTokens m_tokens;
    CreateConnectionUseCase m_createConnection{m_config, m_tokens};
    CreateResultFileUseCase m_createResultFile;
    TimerUseCase m_timer{m_tokens};
    ChangeableParamsUseCase m_changeableParams;
    SendEmailUseCase m_email;
};

} // namespace app
