#pragma once

#include "model.h"

#include <random>
#include <chrono>

namespace app
{

/**
 * @brief      Класс соединения
 */
class Connection
{
    enum class TimeExpired
    {
        DURATION = 10,
    };
public:
    Connection(const model::Contract::Id id, const size_t duration);
    model::Contract::Id GetContractId() const noexcept;
    size_t GetContractDuration() const noexcept;
    bool IsExpired(const std::chrono::steady_clock::time_point& timeNow) const;
private:
    model::Contract::Id m_id;
    size_t m_duration{1};
    std::chrono::steady_clock::time_point m_time;
};

namespace detail
{
    /**
     * @brief      Тэг токена
     */
    struct TokenTag
    {
    };
} // namesapce detail

using Token = util::Tagged<std::string, detail::TokenTag>;

/**
 * @brief      Класс токенов для соединений
 */
class ConnectionTokens
{
    using TokenHasher = util::TaggedHasher<Token>;
    using TokenToConnection = std::unordered_map<Token, std::unique_ptr<Connection>, TokenHasher>;
public:
    Token AddConnection(Connection connection);
    Connection* FindConnectionBy(const Token& token) const noexcept;
    void Tick(const std::chrono::steady_clock::time_point& timeNow);
private:
    Token GenerateToken();

    TokenToConnection m_tokenToConn;
    std::random_device m_device;
    std::mt19937_64 m_gen1{[this]{
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(m_device);
    }()};
    std::mt19937_64 m_gen2{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(m_device);
    }()};
};

} // namespace app
