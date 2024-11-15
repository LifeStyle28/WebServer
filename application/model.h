#pragma once

#include <vector>
#include <memory>
#include <string_view>

#include <boost/iterator/indirect_iterator.hpp>

namespace model
{

struct ContractField
{
    std::string m_key;
    std::string m_value;
    std::string m_tag;
};

class Contract
{
public:
    using ContractFields = std::vector<ContractField>;

    Contract() = default;
    void AddContractField(ContractField contractField);
    ContractFields GetContractFields() const noexcept;
private:
    ContractFields m_contractFields;
};

class Config
{
    using ContractsStorage = std::vector<std::unique_ptr<Contract>>;
public:
    class ContractsView
    {
    public:
        using StoragePtr = const ContractsStorage*;
        using Iterator = boost::indirect_iterator<ContractsStorage::const_iterator>; ///< позволяет итерироваться по контейнеру указателей

        explicit ContractsView(StoragePtr storage) noexcept;
        size_t Size() const noexcept;
        Iterator begin() const noexcept; ///< для range based for
        Iterator end() const noexcept; ///< для range based for
    private:
        StoragePtr m_storage;
    };

    Config() = default;
    ~Config() = default;
    void AddField(Contract contract);
    ContractsView GetContracts() const noexcept;
private:
    ContractsStorage m_contracts;
};

} // namespace model
