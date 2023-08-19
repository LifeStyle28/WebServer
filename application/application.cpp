#include "application.h"

namespace app
{

namespace fs = std::filesystem;

Application::Application(model::Config& config, const fs::path& scriptPath, const fs::path& resultPath) :
    m_config{config},
    m_createResultFile{scriptPath, resultPath}
{
}

const model::Contract::ContractTagValues& Application::GetTagValues(const model::Contract::Id id) const
{
    return m_bringTagValues.GetTagValues(id);
}

std::string Application::GetResultFileName(const std::string& body) const
{
    return m_createResultFile.CreateFile(body);
}

} // namespace app
