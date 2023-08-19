#pragma once

#include "model.h"

#include <variant>
#include <filesystem>

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

class CreateResultFileUseCase
{
public:
    CreateResultFileUseCase(std::filesystem::path scriptPath, std::filesystem::path resultPath);
    std::string CreateFile(const std::string& body) const;
private:
    std::filesystem::path m_scriptPath;
    std::filesystem::path m_resultPath;
};

} // namespace app
