#include "model.h"

namespace model
{

void Contract::AddContractField(ContractField contractField)
{
    m_contractFields.emplace_back(std::move(contractField));
}

Contract::ContractFields Contract::GetContractFields() const noexcept
{
    return m_contractFields;
}

Config::ContractsView::ContractsView(Config::ContractsView::StoragePtr storage) noexcept :
    m_storage{storage}
{
}

size_t Config::ContractsView::Size() const noexcept
{
    return m_storage->size();
}

Config::ContractsView::Iterator Config::ContractsView::begin() const noexcept
{
    return m_storage->begin();
}

Config::ContractsView::Iterator Config::ContractsView::end() const noexcept
{
    return m_storage->end();
}

void Config::AddField(Contract contract)
{
    m_contracts.emplace_back(std::make_unique<Contract>(std::move(contract)));
}

Config::ContractsView Config::GetContracts() const noexcept
{
    return ContractsView{&m_contracts};
}

} // namespace model
