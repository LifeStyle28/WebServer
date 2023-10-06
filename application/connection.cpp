#include "connection.h"

#include <iomanip>

namespace app
{

/**
 * @brief      Конструктор
 *
 * @param[in]  id        идентификатор контракта
 * @param[in]  duration  длительность контракта (лет)
 */
Connection::Connection(const model::Contract::Id id, const size_t duration) :
    m_id{id},
    m_duration{duration},
    m_time{std::chrono::steady_clock::now()}
{
}

/**
 * @brief      Получить идентификатор контракта
 *
 * @return     Идентификатор контракта
 */
model::Contract::Id Connection::GetContractId() const noexcept
{
    return m_id;
}

/**
 * @brief      Получить длительность контракта
 *
 * @return     Длительность контракта
 */
size_t Connection::GetContractDuration() const noexcept
{
    return m_duration;
}

/**
 * @brief      Проверка, вышло ли время сессии текущего пользователя
 *
 * @param[in]  timeNow  текущее время
 *
 * @return     true - если время вышло, false - если нет
 */
bool Connection::IsExpired(const std::chrono::steady_clock::time_point& timeNow) const
{
    using namespace std::chrono;
    if (duration_cast<minutes>(timeNow - m_time).count() > static_cast<int>(TimeExpired::DURATION))
    {
        return true;
    }
    return false;
}

/**
 * @brief      Добавить новое соединение
 *
 * @param[in]  connection  соединение
 *
 * @return     Указатель на соединение или nullptr
 */
Token ConnectionTokens::AddConnection(Connection connection)
{
    Token token = GenerateToken();
    m_tokenToConn.emplace(token, std::make_unique<Connection>(std::move(connection)));
    return token;
}

Connection* ConnectionTokens::FindConnectionBy(const Token& token) const noexcept
{
    if (auto it = m_tokenToConn.find(token); it != m_tokenToConn.end())
    {
        return it->second.get();
    }
    return nullptr;
}

/**
 * @brief      Метод обработки тика таймера (удаление просроченного соединения)
 *
 * @param[in]  timeNow  текущее время
 */
void ConnectionTokens::Tick(const std::chrono::steady_clock::time_point& timeNow)
{
    auto it = m_tokenToConn.begin();
    for (; it != m_tokenToConn.end(); ++it)
    {
        if (it->second->IsExpired(timeNow))
        {
            it = m_tokenToConn.erase(it);
        }
    }
}

/**
 * @brief      Сгенерировать токен текущего пользователя
 *
 * @return     Токен текущего пользователя
 */
Token ConnectionTokens::GenerateToken()
{
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream out;
    out << std::setw(16) << std::setfill('0')
        << std::hex << dist(m_gen1)
        << std::setw(16) << dist(m_gen2);
    return Token{std::move(out).str()};
}

} // namespace app
