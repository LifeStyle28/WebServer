#pragma once

#include "tagged.h"

#include <vector>
#include <string_view>
#include <unordered_map>

#include <boost/iterator/indirect_iterator.hpp>

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

class Config
{
    using ContractsStorage = std::vector<std::unique_ptr<Contract>>;
    using ContractIdHasher = util::TaggedHasher<Contract::Id>;
    using ContractIdToIndex = std::unordered_map<Contract::Id, size_t, ContractIdHasher>;
public:
    Config() = default;
    Config(Config&& other) noexcept(false); ///< т.к. имеем unique_ptr
    Config& operator=(Config&& rhs) noexcept(false); ///< т.к. имеем unique_ptr
    ~Config() = default;
    void AddContract(Contract contract);
    Contract* FindContractIndexBy(const Contract::Id id) const noexcept;
private:
    ContractsStorage m_contracts;
    ContractIdToIndex m_contractIdToIndex;
};

} // namespace model
