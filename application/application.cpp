#include "application.h"

namespace app
{

namespace fs = std::filesystem;

Application::Application(model::Config& config, const fs::path& scriptPath,
    const fs::path& resultPath, const fs::path& webPath, const fs::path& configJsonPath) :
        m_config{config},
        m_createResultFile{scriptPath, resultPath, webPath, m_config, m_tokens},
        m_changePercent{m_config, configJsonPath}
{
}

CreateConnectionResult Application::CreateConnection(const model::Contract::Id id,
    const size_t duration) const
{
    return m_createConnection.CreateConnection(id, duration);
}

std::string Application::GetResultFileName(const std::string& body, const Token& token) const
{
    return m_createResultFile.CreateFile(body, token);
}

void Application::Tick(const std::chrono::steady_clock::time_point& timeNow)
{
    m_timer.Tick(timeNow);
}

void Application::ChangePercent(const size_t percent)
{
    m_changePercent.ChangePercent(percent);
}

} // namespace app
