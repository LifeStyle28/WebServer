#pragma once

#include "use_cases.h"

namespace app
{

class Application
{
public:
    Application(model::Config& config, const std::filesystem::path& scriptPath,
        const std::filesystem::path& resultPath);
    const model::Contract::ContractTagValues& GetTagValues(const model::Contract::Id id) const;
    std::string GetResultFileName(const std::string& body) const;
private:
    model::Config& m_config;
    BringTagValuesUseCase m_bringTagValues{m_config};
    CreateResultFileUseCase m_createResultFile;
};

} // namespace app
