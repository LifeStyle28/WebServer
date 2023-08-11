#include "use_cases.h"

namespace app
{

BringTagValuesError::BringTagValuesError(Reason reason) :
    std::runtime_error{"Failed to bring tag values"},
    m_reason{reason}
{
}

const BringTagValuesError::Reason& BringTagValuesError::GetReason() const
{
    return m_reason;
}

BringTagValuesUseCase::BringTagValuesUseCase(std::reference_wrapper<const model::Config> config) :
    m_config{config.get()}
{
}

const model::Contract::ContractTagValues& BringTagValuesUseCase::GetTagValues(const model::Contract::Id id) const
{
    const auto contractPtr{m_config.FindContractIndexBy(id)};
    if (contractPtr)
    {
        return contractPtr->GetContractTagValues();
    }
    throw BringTagValuesError{BringTagValuesError::InvalidPointer{}};
}

} // namespace app
