#include "use_cases.h"
#include "boost_logger.h"
#include "json_loader.h"

#include <fstream>

#pragma GCC diagnostic ignored "-Wunused-result" // @TODO - ugly

namespace app
{

using namespace boost_logger;
using namespace std::literals;
namespace fs = std::filesystem;
namespace json = boost::json;
namespace logging = boost::log;

CreateConnectionError::CreateConnectionError(Reason reason) :
    std::runtime_error{"Failed to bring tag values"s},
    m_reason{reason}
{
}

const CreateConnectionError::Reason& CreateConnectionError::GetReason() const
{
    return m_reason;
}

CreateConnectionUseCase::CreateConnectionUseCase(std::reference_wrapper<const model::Config> config,
    ConnectionTokens& connTokens) :
        m_config{config.get()},
        m_connTokens{connTokens}
{
}

CreateConnectionResult CreateConnectionUseCase::CreateConnection(const model::Contract::Id id,
    std::string_view type, const size_t duration) const
{
    const auto contractPtr{m_config.FindContractIndexBy(id)};
    if (!contractPtr)
    {
        throw CreateConnectionError{CreateConnectionError::InvalidPointer{}};
    }

    try
    {
        const auto token{m_connTokens.AddConnection(Connection{id, type, duration})};
        return {token, contractPtr->GetContractTagValues()};
    }
    catch (...)
    {
    }
    throw CreateConnectionError{CreateConnectionError::OtherReason{}};
}

CreateResultFileUseCase::CreateResultFileUseCase(fs::path scriptPath, fs::path resultPath,
    fs::path webPath, std::reference_wrapper<const model::Config> config,
    const ConnectionTokens& connTokens) :
        m_scriptPath{scriptPath},
        m_resultPath{resultPath},
        m_webPath{webPath},
        m_config{config.get()},
        m_connTokens{connTokens}
{
}

/**
 * @brief      Makes a directory.
 *
 * @param[in]  savePath  The save path
 */
static void make_dir(const std::string& savePath)
{
    std::stringstream command;
    command << "mkdir -p "sv;
    command << savePath;
    if (std::system(command.str().c_str()) != 0)
    {
        throw std::runtime_error{"Failed to create folder"s};
    }
}

/**
 * @brief      Исполняет python-скрипт
 *
 * @param[in]  savePath    The save path
 * @param[in]  jsonConfig  The json configuration
 * @param[in]  scriptPath  The script path
 */
static void call_script(const std::string& savePath, const std::string& jsonConfig,
    const std::string& scriptPath, const std::string& docNums, size_t contractDuration,
    const size_t percent)
{
    std::stringstream command;
    command << "python3 "sv;
    command << '\'' << scriptPath << '\'' << ' '; ///< script path
    command << '\'' << jsonConfig << '\'' << ' '; ///< json-string
    command << '\'' << savePath << '\'' << ' '; ///< path for save
    command << '\'' << docNums << '\'' << ' ';
    command << '\'' << contractDuration << '\'' << ' ';
    command << '\'' << percent << '\'';
    if (std::system(command.str().c_str()) != 0)
    {
        throw std::runtime_error{"Failed to make documents with python-script"s};
    }
}

/**
 * @brief      Запускает py-script
 *
 * @param[in]  savePath          директория для сохранения
 * @param[in]  jsonConfig        файл конфигурации json
 * @param[in]  scriptPath        директория скрипта
 * @param[in]  docNums           номера документов
 * @param[in]  contractDuration  длительность контракта
 * @param[in]  percent           процент
 */
static void start_script(const std::string& savePath, const std::string& jsonConfig,
    const std::string& scriptPath, const std::string& docNums, size_t contractDuration,
    const size_t percent)
{
    make_dir(savePath); ///< создает папку для результатов
    call_script(savePath, jsonConfig, scriptPath, docNums, contractDuration, percent); ///< вызывает скрипт
}

std::string CreateResultFileUseCase::CreateFile(const std::string& body, const Token& token) const
{
    const auto connection{m_connTokens.FindConnectionBy(token)};
    if (!connection)
    {
        throw AuthorizationError{"Connection token has not been found"s};
    }

    std::stringstream docNums;
    for (const auto& i : m_config.FindContractIndexBy(connection->GetContractId())->GetDocumentsNumbers())
    {
        docNums << i << ' ';
    }

    try
    {
        std::random_device random_device;
        std::mt19937_64 generator{[&random_device] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device);
        }()};
        std::uniform_int_distribution<uint64_t> dist;
        const std::string folderName(std::to_string(dist(generator))); ///< название сгенерированной папки
        const std::string generatedFolderPath(m_resultPath.string() + folderName + "/"s); ///< полный путь сгенерированной папки

        start_script(generatedFolderPath, body, m_scriptPath, docNums.str(),
            connection->GetContractDuration(), m_config.GetPercent(connection->GetPercentType()));

        std::string path;
        for (const auto& entry : fs::directory_iterator(generatedFolderPath))
        {
            if (entry.path().string().find(".7z"s) != std::string::npos)
            {
                path = entry.path().string();
                break;
            }
        }
        if (path.empty())
        {
            throw std::runtime_error("Can't find 7-zip file"s);
        }
        return path;
    }
    catch (const std::exception& ex)
    {
        json::value data{{"exception"s, ex.what()}};
        BOOST_LOG_TRIVIAL(error) << logging::add_value(additional_data, data);
        // @TODO отловить ошибки, которые реально могут возникнуть
    }
    throw std::runtime_error("Can't create file"s);
}

std::string CreateResultFileUseCase::MakeResultPath(const std::string& path) const
{
    auto make_result_path = [&path](fs::path webPath)
    {
        if (const auto pos = path.find(webPath.string()); pos != std::string::npos)
        {
            return path.substr(pos + webPath.string().size());
        }
        return path;
    };

    return make_result_path(m_webPath);
}

TimerUseCase::TimerUseCase(ConnectionTokens& connTokens) :
    m_connTokens{connTokens}
{
}

void TimerUseCase::Tick(const std::chrono::steady_clock::time_point& timeNow)
{
    m_connTokens.Tick(timeNow);
}

ChangeableParamsUseCase::ChangeableParamsUseCase(model::Config& config, fs::path configJsonPath) :
    m_config{config},
    m_configJsonPath{configJsonPath}
{
}

void ChangeableParamsUseCase::ChangeParams(model::Config::PercentsArray percents, std::string email)
{
    m_config.SetPercents(percents);
    m_config.SetEmail(email);

    try
    {
        auto jsonString{json_loader::load_file_as_string(m_configJsonPath)};
        auto jsonValue = json::parse(jsonString);

        for (size_t i = 0; i < static_cast<size_t>(model::PercentType::PERCENT_CNT); ++i)
        {
            auto itPercent{jsonValue.as_object().find(
                convert_percent_type_to_sv(static_cast<model::PercentType>(i)))};
            if (itPercent != jsonValue.as_object().end())
            {
                itPercent->value().as_int64() = percents[i];
            }
        }

        auto itEmail{jsonValue.as_object().find(json_loader::ConfigToken::EMAIL)};
        if (itEmail != jsonValue.as_object().end())
        {
            itEmail->value().as_string() = std::move(email);
        }

        std::ofstream out{m_configJsonPath};
        if (out.is_open())
        {
            out << json::serialize(jsonValue);
        }
    }
    catch (const std::exception& e)
    {
        json::value data{{"exception"s, e.what()}};
        BOOST_LOG_TRIVIAL(error) << logging::add_value(additional_data, data);
        throw ChangeParamsError{"Params saved in RAM. Try again to save it to ROM."s};
    }
}

size_t ChangeableParamsUseCase::GetPercent(const model::PercentType type) const noexcept
{
    return m_config.GetPercent(type);
}

std::string_view ChangeableParamsUseCase::GetEmail() const noexcept
{
    return m_config.GetEmail();
}

SendEmailUseCase::SendEmailUseCase(std::reference_wrapper<const model::Config> config, fs::path webPath) :
    m_config{config.get()},
    m_webPath{webPath}
{
}

void SendEmailUseCase::SendEmail(std::string_view filePath) const
{
    const auto fileName = filePath.substr(filePath.find_last_of('/') + 1);
    {
        json::value data{{"fileName"s, filePath}};
        BOOST_LOG_TRIVIAL(debug) << logging::add_value(additional_data, data);
    }

    const auto email = m_config.GetEmail();

    if (!email.empty())
    {
        auto command = "mutt -s \""s + std::string(fileName) + "\" -a"s + std::string(filePath)
            + " -- "s + email;

        auto pos = command.find('$');

        while (pos != std::string::npos)
        {
            command.insert(pos, 1, '\\');
            pos = command.find('$', pos + 2);
        }

        {
            json::value data{{"command"s, command}};
            BOOST_LOG_TRIVIAL(debug) << logging::add_value(additional_data, data);
        }

        [[maybe_unused]] const int result = std::system(command.c_str());
    }
}

} // namespace app
