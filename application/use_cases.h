#pragma once

#include "model.h"

namespace app
{

class BringFieldsUseCase
{
public:
    BringFieldsUseCase(const model::Config::ContractsView& contracts);
    model::Contract::ContractFields GetFields([[maybe_unused]] size_t index) const noexcept;
private:
};

} // namespace app
