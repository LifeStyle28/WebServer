#include "use_cases.h"

namespace app
{

BringFieldsUseCase::BringFieldsUseCase(const model::Config::ContractsView& contracts)
{
}

model::Contract::ContractFields BringFieldsUseCase::GetFields(
    [[maybe_unused]] size_t index) const noexcept
{
    return model::Contract::ContractFields();
}

} // namespace app
