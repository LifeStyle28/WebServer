#pragma once

#include "tagged.h"

#include <vector>
#include <string_view>
#include <unordered_map>
#include <array>

#include <boost/utility/string_view.hpp>

namespace model
{

struct ContractTagValue
{
    std::string m_key;
    std::string m_tag;
    std::string m_value;
};

class Contract
{
public:
    using Id = util::Tagged<std::string, Contract>;
    using DocumentsNumbers = std::vector<size_t>;
    using ContractTagValues = std::vector<ContractTagValue>;

    Contract(Id id);
    void AddContractTagValue(ContractTagValue contractField);
    void AddDocNum(const size_t num);
    Id GetId() const noexcept;
    const DocumentsNumbers& GetDocumentsNumbers() const noexcept;
    const ContractTagValues& GetContractTagValues() const noexcept;
private:
    Id m_id;
    DocumentsNumbers m_docsNums;
    ContractTagValues m_contractFields;
};

enum class PercentType
{
    PERCENT_ROUBLES = 0,
    PERCENT_USD,
    PERCENT_EURO,
    PERCENT_USDT,
    PERCENT_CNT
};

class Config
{
    using ContractsStorage = std::vector<std::unique_ptr<Contract>>;
    using ContractIdHasher = util::TaggedHasher<Contract::Id>;
    using ContractIdToIndex = std::unordered_map<Contract::Id, size_t, ContractIdHasher>;
public:
    using PercentsArray = std::array<size_t, static_cast<size_t>(PercentType::PERCENT_CNT)>;
public:
    Config() = default;
    Config(Config&& other) noexcept(false); ///< т.к. имеем unique_ptr
    Config& operator=(Config&& rhs) noexcept(false); ///< т.к. имеем unique_ptr
    ~Config() = default;
    void AddContract(Contract contract);
    Contract* FindContractIndexBy(const Contract::Id id) const noexcept;
    void SetPercents(PercentsArray percents);
    size_t GetPercent(const PercentType type) const noexcept;
    void SetEmail(std::string email);
    const std::string& GetEmail() const noexcept;
private:
    ContractsStorage m_contracts;
    ContractIdToIndex m_contractIdToIndex;
    PercentsArray m_percents;
    std::string m_email;
};

boost::string_view convert_percent_type_to_sv(const PercentType type);

} // namespace model
