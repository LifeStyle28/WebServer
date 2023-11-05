#include "json_loader.h"
#include "boost_logger.h"

#include <fstream>

#include <boost/json/object.hpp>
#include <boost/json/parse.hpp>

namespace json_loader
{

using namespace model;
using namespace std::literals;
namespace json = boost::json;
namespace fs = std::filesystem;
using namespace boost_logger;
namespace logging = boost::log;

namespace
{

struct Tool
{
    Tool() = delete;
    static constexpr boost::string_view CONTRACTS{"contracts"};
    static constexpr boost::string_view ID{"id"};
    static constexpr boost::string_view DOCS{"docs"};
    static constexpr boost::string_view TAG_VALUES{"tag_values"};
    static constexpr boost::string_view KEY{"key"};
    static constexpr boost::string_view TAG{"tag"};
    static constexpr boost::string_view VALUE{"value"};
};

enum class DefaultInvariants
{
    PERCENT = 22
};

/**
 * @brief      конвертация из json value в std::string
 *
 * @param[in]  val   значение
 *
 * @return     сконвертированное значение
 */
std::string json_val_as_string(const json::value& val)
{
    return std::string(val.as_string());
}

/**
 * Загружает поля контракта
 *
 * @param[in]  tagValue  поле, включающее в себя поля key, tag и value
 *
 * @return     tagValue
 */
ContractTagValue load_tag_value(const json::value& tagValue)
{
    ContractTagValue result;
    if (auto it = tagValue.as_object().find(Tool::KEY); it != tagValue.as_object().end())
    {
        result.m_key = it->value().as_string();
    }
    if (auto it = tagValue.as_object().find(Tool::TAG); it != tagValue.as_object().end())
    {
        result.m_tag = it->value().as_string();
    }
    if (auto it = tagValue.as_object().find(Tool::VALUE); it != tagValue.as_object().end())
    {
        result.m_value = it->value().as_string();
    }
    return result;
}

/**
 * Загружает контракты шаблона
 *
 * @param[in]  obj   json-объект
 *
 * @return     возвращает контракт
 */
Contract load_contract(const json::object& obj)
{
    Contract contract{Contract::Id{json_val_as_string(obj.at(Tool::ID))}};

    for (const json::value& tagValue : obj.at(Tool::TAG_VALUES).as_array())
    {
        contract.AddContractTagValue(load_tag_value(tagValue));
    }

    for (const json::value& docNum : obj.at(Tool::DOCS).as_array())
    {
        contract.AddDocNum(docNum.as_int64());
    }

    return contract;
}

/**
 * @brief      Загружает контракты
 *
 * @param[in]  array  массив для парсинга
 *
 * @return     список возможных контрактов
 */
std::vector<Contract> load_contracts(const json::array& array)
{
    std::vector<Contract> contracts;
    contracts.reserve(array.size());

    for (const auto& contractVal : array)
    {
        contracts.emplace_back(load_contract(contractVal.as_object()));
    }

    return contracts;
}

/**
 * @brief      Загрузить процент
 *
 * @param[in]  val   значение для парсинга
 *
 * @return     процент в size_t
 */
size_t load_percent(const json::value& val)
{
    return val.as_int64();
}

/**
 * Загружает шаблон
 *
 * @param      obj   json-объект
 *
 * @return     конфиг
 */
Config load_config(const json::object& obj)
{
    const auto contracts = load_contracts(obj.at(Tool::CONTRACTS).as_array());
    Config config;
    for (const auto& contract : contracts)
    {
        config.AddContract(contract);
    }

    size_t percent = static_cast<size_t>(DefaultInvariants::PERCENT);
    if (auto it = obj.find(ConfigToken::PERCENT); it != obj.end())
    {
        percent = load_percent(it->value());
    }

    {
        json::value data{{"percent"s, percent}};
        BOOST_LOG_TRIVIAL(debug) << logging::add_value(additional_data, data);
    }

    config.SetPercent(percent);

    return config;
}

} // anonymous namespace

/**
 * @brief      Загрузить файл в строку
 *
 * @param[in]  jsonPath  путь файла конфигурации json
 *
 * @return     json-файл в виде строки
 */
std::string load_file_as_string(const fs::path& jsonPath)
{
    std::ifstream jsonStream{jsonPath};
    if (!jsonStream)
    {
        throw std::runtime_error{"Failed to open file "s + jsonPath.string()};
    }
    std::stringstream jsonStringStream;
    if (!(jsonStringStream << jsonStream.rdbuf()))
    {
        throw std::runtime_error{"Failed to read content"s};
    }
    return std::move(jsonStringStream).str();
}

Config load_config(const fs::path& jsonPath)
{
    auto jsonString = load_file_as_string(jsonPath);
    auto jsonValue = json::parse(jsonString);
    return load_config(jsonValue.as_object());
}

} // namespace json_loader
