#include "use_cases.h"

#include <random>

#pragma GCC diagnostic ignored "-Wunused-result" // @TODO - ugly

namespace app
{

namespace fs = std::filesystem;

#define DOCS_ZIP "docs.zip"

CreateConnectionError::CreateConnectionError(Reason reason) :
    std::runtime_error{"Failed to bring tag values"},
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
    const size_t duration) const
{
    const auto contractPtr{m_config.FindContractIndexBy(id)};
    if (!contractPtr)
    {
        throw CreateConnectionError{CreateConnectionError::InvalidPointer{}};
    }

    try
    {
        const auto token{m_connTokens.AddConnection(Connection{id, duration})};
        return {token, contractPtr->GetContractTagValues()};
    }
    catch (...)
    {
    }
    throw CreateConnectionError{CreateConnectionError::OtherReason{}};
}

CreateResultFileUseCase::CreateResultFileUseCase(fs::path scriptPath, fs::path resultPath,
    std::reference_wrapper<const model::Config> config, const ConnectionTokens& connTokens) :
        m_scriptPath{scriptPath},
        m_resultPath{resultPath},
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
    command << "mkdir -p ";
    command << savePath;
    if (std::system(command.str().c_str()) != 0)
    {
        throw std::runtime_error{"Failed to create folder"};
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
    const std::string& scriptPath, const std::string& docNums, size_t contractDuration)
{
    std::stringstream command;
    command << "python3 ";
    command << '\'' << scriptPath << '\'' << ' '; ///< script path
    command << '\'' << jsonConfig << '\'' << ' '; ///< json-string
    command << '\'' << savePath << '\'' << ' '; ///< path for save
    command << '\'' << docNums << '\'' << ' ';
    command << '\'' << contractDuration << '\'';
    if (std::system(command.str().c_str()) != 0)
    {
        throw std::runtime_error{"Failed to make documents with python-script"};
    }
}

/**
 * @brief      Removes an old archive.
 *
 * @param[in]  savePath  The save path
 */
static void remove_old_archive(const std::string& savePath)
{
    std::stringstream command;
    command << "rm -rf ";
    command << savePath;
    command << DOCS_ZIP;
    std::system(command.str().c_str());
}

/**
 * @brief      Makes an archive.
 *
 * @param[in]  savePath  The save path
 */
static void make_archive(const std::string& savePath)
{
    std::stringstream command;
    command << "zip -r ";
    command << savePath;
    command << DOCS_ZIP;
    command << " ";
    command << savePath;
    if (std::system(command.str().c_str()) != 0)
    {
        throw std::runtime_error{"Failed to create tar"};
    }
}

//@ TODO завернуть в strand
static void start_script(const std::string& savePath, const std::string& jsonConfig,
    const std::string& scriptPath, const std::string& docNums, size_t contractDuration)
{
    make_dir(savePath); ///< создает папку для результатов
    call_script(savePath, jsonConfig, scriptPath, docNums, contractDuration); ///< вызывает скрипт
    remove_old_archive(savePath); ///< удаляет старый архив, если он есть
    make_archive(savePath); ///< упаковывает документы в архив
}

std::string CreateResultFileUseCase::CreateFile(const std::string& body, const Token& token) const
{
    const auto connection{m_connTokens.FindConnectionBy(token)};
    if (!connection)
    {
        throw AuthorizationError{"Connection token has not been found"};
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
        const std::string generatedFolderPath(m_resultPath.string() + folderName + "/"); ///< полный путь сгенерированной папки
        const std::string path = generatedFolderPath + '/' + DOCS_ZIP;
        start_script(generatedFolderPath, body, m_scriptPath, docNums.str(), connection->GetContractDuration()); ///< @TODO проверить пути и работу скрипта
        return path;
    }
    catch (const std::exception& e)
    {
        // @TODO отловить ошибки, которые реально могут возникнуть
    }
    throw std::runtime_error("Can't create file");
}

TimerUseCase::TimerUseCase(ConnectionTokens& connTokens) :
    m_connTokens{connTokens}
{
}

void TimerUseCase::Tick(const std::chrono::steady_clock::time_point& timeNow)
{
    m_connTokens.Tick(timeNow);
}

} // namespace app
