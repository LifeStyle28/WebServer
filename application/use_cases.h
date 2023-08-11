#pragma once

#include "model.h"

#include <variant>

namespace app
{

class BringTagValuesError : public std::runtime_error
{
public:
    struct InvalidPointer
    {
    };
    struct Dummy ///< для каких-либо будущих ошибок, чтобы заюзать вариант
    {
    };
    using Reason = std::variant<InvalidPointer, Dummy>;

    BringTagValuesError(Reason reason);
    const Reason& GetReason() const;
private:
    Reason m_reason;
};

class BringTagValuesUseCase
{
public:
    BringTagValuesUseCase(std::reference_wrapper<const model::Config> config);
    const model::Contract::ContractTagValues& GetTagValues(const model::Contract::Id id) const;
private:
    const model::Config& m_config;
};

} // namespace app
