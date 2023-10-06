#pragma once

#include "connection.h"

#include <variant>
#include <filesystem>

namespace app
{

class CreateConnectionError : public std::runtime_error
{
public:
    struct InvalidPointer
    {
    };
    struct OtherReason
    {
    };
    using Reason = std::variant<InvalidPointer, OtherReason>;

    CreateConnectionError(Reason reason);
    const Reason& GetReason() const;
private:
    Reason m_reason;
};

struct CreateConnectionResult
{
    Token m_token;
    const model::Contract::ContractTagValues& m_tagValues;
};

class CreateConnectionUseCase
{
public:
    CreateConnectionUseCase(std::reference_wrapper<const model::Config> config,
        ConnectionTokens& connTokens);
    CreateConnectionResult CreateConnection(const model::Contract::Id id, const size_t duration) const;
private:
    const model::Config& m_config;
    ConnectionTokens& m_connTokens;
};

class AuthorizationError : public std::domain_error
{
public:
    using domain_error::domain_error;
};

class CreateResultFileUseCase
{
public:
    CreateResultFileUseCase(std::filesystem::path scriptPath, std::filesystem::path resultPath,
        std::reference_wrapper<const model::Config> config, const ConnectionTokens& connTokens);
    std::string CreateFile(const std::string& body, const Token& token) const;
private:
    std::filesystem::path m_scriptPath;
    std::filesystem::path m_resultPath;
    const model::Config& m_config;
    const ConnectionTokens& m_connTokens;
};

class TimerUseCase
{
public:
    TimerUseCase(ConnectionTokens& connTokens);
    void Tick(const std::chrono::steady_clock::time_point& timeNow);
private:
    ConnectionTokens& m_connTokens;
};

} // namespace app
