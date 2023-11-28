#include "application.h"

#include <iostream>

namespace app
{

namespace fs = std::filesystem;

Application::Application(model::Config& config, const fs::path& scriptPath,
    const fs::path& resultPath, const fs::path& webPath, const fs::path& configJsonPath) :
        m_config{config},
        m_createResultFile{scriptPath, resultPath, webPath, m_config, m_tokens},
        m_changeableParams{m_config, configJsonPath},
        m_email{m_config, webPath}
{
}

CreateConnectionResult Application::CreateConnection(const model::Contract::Id id,
    const size_t duration) const
{
    return m_createConnection.CreateConnection(id, duration);
}

std::string Application::GetResultFileName(const std::string& body, const Token& token) const
{
    const auto path = m_createResultFile.CreateFile(body, token);
    m_email.SendEmail(path);
    return m_createResultFile.MakeResultPath(path);
}

void Application::Tick(const std::chrono::steady_clock::time_point& timeNow)
{
    m_timer.Tick(timeNow);
}

void Application::ChangeContractParams(const size_t percent, std::string email)
{
    m_changeableParams.ChangeParams(percent, std::move(email));
}

size_t Application::GetContractPercent() const noexcept
{
    return m_changeableParams.GetPercent();
}

std::string_view Application::GetContractEmail() const noexcept
{
    return m_changeableParams.GetEmail();
}

} // namespace app
