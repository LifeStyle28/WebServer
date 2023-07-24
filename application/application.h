#pragma once

#include "use_cases.h"

namespace app
{

class Application
{
public:
    Application(model::Config& config);
private:
    model::Config& m_config;
    BringFieldsUseCase m_bringFields{m_config.GetContracts()};
};

} // namespace app
