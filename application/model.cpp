#include "model.h"

#include <unordered_map>

#include <boost/scope_exit.hpp>

namespace model
{

using namespace std::literals;

Contract::Contract(Id id) :
    m_id{id}
{
}

void Contract::AddContractTagValue(ContractTagValue contractField)
{
    m_contractFields.emplace_back(std::move(contractField));
}

void Contract::AddDocNum(const size_t num)
{
    m_docsNums.emplace_back(num);
}

Contract::Id Contract::GetId() const noexcept
{
    return m_id;
}

const Contract::DocumentsNumbers& Contract::GetDocumentsNumbers() const noexcept
{
    return m_docsNums;
}

const Contract::ContractTagValues& Contract::GetContractTagValues() const noexcept
{
    return m_contractFields;
}

Config::Config(Config&& other) noexcept(false) :
    m_contracts{std::move(other.m_contracts)},
    m_contractIdToIndex{std::move(other.m_contractIdToIndex)}
{
    other.m_contracts.clear();
    other.m_contractIdToIndex.clear();
}

Config& Config::operator=(Config&& rhs) noexcept(false)
{
    m_contracts.swap(rhs.m_contracts);
    m_contractIdToIndex.swap(rhs.m_contractIdToIndex);
    return *this;
}

void Config::AddContract(Contract contract)
{
    const size_t index{m_contracts.size()};
    if (auto [it, inserted] = m_contractIdToIndex.emplace(contract.GetId(), index); !inserted)
    {
        throw std::invalid_argument{"Contract with id "s + *contract.GetId() + " already exists"s};
    }
    else
    {
        // Захватим весь скоуп, чтобы, если что удалить контракт из хеш-мапы.
        // Вроде с С++11 работает, сработает при выходе из else, а так же при выбросе исключения
        bool commit = false;
        BOOST_SCOPE_EXIT_ALL(&commit, it, this)
        {
            if (!commit)
            {
                m_contractIdToIndex.erase(it);
            }
        };

        m_contracts.emplace_back(std::make_unique<Contract>(std::move(contract)));
        commit = true;
    }
}

Contract* Config::FindContractIndexBy(const Contract::Id id) const noexcept
{
    if (auto it = m_contractIdToIndex.find(id); it != m_contractIdToIndex.end())
    {
        return m_contracts.at(it->second).get();
    }
    return nullptr;
}

/**
 * @brief      Установка процента по договору
 *
 * @param[in]  percent  процент
 */
void Config::SetPercents(PercentsArray percents)
{
    m_percents = std::move(percents);
}

/**
 * @brief      Возвращает процент
 *
 * @return     процент
 */
size_t Config::GetPercent(const PercentType type) const noexcept
{
    return m_percents[static_cast<size_t>(type)];
}

/**
 * @brief      Установить адрес почты агрегации документов
 *
 * @param[in]  email  адрес почты
 */
void Config::SetEmail(std::string email)
{
    m_email = std::move(email);
}

/**
 * @brief      Получить адрес почты агрегации документов
 *
 * @return     адрес почты
 */
const std::string& Config::GetEmail() const noexcept
{
    return m_email;
}

/**
 * @brief      Сконвертировать тип процента в строку конфигурации
 *
 * @param[in]  type  тип
 *
 * @return     строка конфигурации
 */
boost::string_view convert_percent_type_to_sv(const PercentType type)
{
    static const std::unordered_map<PercentType, boost::string_view> typeToString =
    {
        {PercentType::PERCENT_ROUBLES, "percent_roubles"},
        {PercentType::PERCENT_USD, "percent_usd"},
        {PercentType::PERCENT_EURO, "percent_euro"},
        {PercentType::PERCENT_USDT, "percent_usdt"},
    };

    if (type < PercentType::PERCENT_CNT)
    {
        return typeToString.at(type);
    }
    throw std::runtime_error{"Bad percent type"s};
}

} // namespace model
