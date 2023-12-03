#include "connection.h"

#include <iomanip>

namespace app
{

using namespace std::literals;

/**
 * @brief      Сконвертировать тип валюты в тип процентной ставки
 *
 * @param[in]  type  тип валюты
 *
 * @return     тип процентной ставки
 */
static model::PercentType convert_string_to_percent_type(std::string_view type)
{
    static const std::unordered_map<std::string_view, model::PercentType> svToPercentType =
    {
        {"ROUBLES"sv, model::PercentType::PERCENT_ROUBLES},
        {"DOLLARS"sv, model::PercentType::PERCENT_USD},
        {"EURO"sv, model::PercentType::PERCENT_EURO},
        {"USDT"sv, model::PercentType::PERCENT_USDT},
    };

    if (auto it = svToPercentType.find(type); it != svToPercentType.end())
    {
        return it->second;
    }
    throw std::runtime_error{"Bad percent type string"s};
}

/**
 * @brief      Конструктор
 *
 * @param[in]  id        идентификатор контракта
 * @param[in]  type      тип процентной валюты
 * @param[in]  duration  длительность контракта (лет)
 */
Connection::Connection(const model::Contract::Id id, std::string_view type, const size_t duration) :
    m_id{id},
    m_type{convert_string_to_percent_type(type)},
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
 * @brief      Получить тип процентной ставки
 *
 * @return     тип процентной ставки
 */
model::PercentType Connection::GetPercentType() const noexcept
{
    return m_type;
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
 * @return     Токен текущего соединения
 */
Token ConnectionTokens::AddConnection(Connection connection)
{
    Token token = GenerateToken();
    m_tokenToConn.emplace(token, std::make_unique<Connection>(std::move(connection)));
    return token;
}

/**
 * @brief      Поиск соединения по токену
 *
 * @param[in]  token  токен
 *
 * @return     Указатель на соединение или nullptr
 */
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
    while (it != m_tokenToConn.end())
    {
        if (it->second->IsExpired(timeNow))
        {
            it = m_tokenToConn.erase(it);
        }
        else
        {
            ++it;
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
