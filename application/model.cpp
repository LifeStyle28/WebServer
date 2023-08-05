#include <fstream>
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

ConfigLoader::ConfigLoader(Config& config) :
    m_config(config)
{
}

/**
 * Загружает шаблон
 * @param in json-шаблон
 */
void ConfigLoader::Load(std::ifstream& in)
{
    const std::string jsonString(std::istreambuf_iterator<char>{in}, {});
    auto values = boost::json::parse(jsonString);

    for (auto& value : values.as_object()) ///< contracts
    {
        m_config.AddField(LoadContract(value.value()));
    }
}

/**
 * Загружает контракты шаблона
 * @param contract контракт
 */
Contract ConfigLoader::LoadContract(boost::json::value& contract)
{
    model::Contract result;
    for (auto& value : contract.as_array()) ///< fields
    {
        result.AddContractField(LoadField(value));
    }
    return result;
}

/**
 * Загружает поля контракта
 * @field поле, включающее в себя поля key, tag и value
 */
ContractField ConfigLoader::LoadField(boost::json::value& field)
{
    model::ContractField result;
    if (auto it = field.as_object().find("key"); it != field.as_object().end())
    {
        result.m_key = it->value().as_string();
    }
    if (auto it = field.as_object().find("tag"); it != field.as_object().end())
    {
        result.m_tag = it->value().as_string();
    }
    if (auto it = field.as_object().find("value"); it != field.as_object().end())
    {
        result.m_value = it->value().as_string();
    }
    return result;
}

} // namespace model
