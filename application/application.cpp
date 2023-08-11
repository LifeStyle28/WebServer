#include "application.h"

namespace app
{

Application::Application(model::Config& config) :
    m_config{config}
{
}

const model::Contract::ContractTagValues& Application::GetTagValues(const model::Contract::Id id) const
{
    return m_bringTagValues.GetTagValues(id);
}

} // namespace app
