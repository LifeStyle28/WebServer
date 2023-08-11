#pragma once

#include "use_cases.h"

namespace app
{

class Application
{
public:
    Application(model::Config& config);
    const model::Contract::ContractTagValues& GetTagValues(const model::Contract::Id id) const;
private:
    model::Config& m_config;
    BringTagValuesUseCase m_bringTagValues{m_config};
};

} // namespace app
