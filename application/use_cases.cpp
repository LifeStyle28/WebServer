#include "use_cases.h"

#include <random>

#pragma GCC diagnostic ignored "-Wunused-result" // @TODO - ugly

namespace app
{

namespace fs = std::filesystem;

#define DOCS_TAR "docs.tar"

BringTagValuesError::BringTagValuesError(Reason reason) :
    std::runtime_error{"Failed to bring tag values"},
    m_reason{reason}
{
}

const BringTagValuesError::Reason& BringTagValuesError::GetReason() const
{
    return m_reason;
}

BringTagValuesUseCase::BringTagValuesUseCase(std::reference_wrapper<const model::Config> config) :
    m_config{config.get()}
{
}

const model::Contract::ContractTagValues& BringTagValuesUseCase::GetTagValues(const model::Contract::Id id) const
{
    // @TODO - сырой указатель - подумать
    const auto contractPtr{m_config.FindContractIndexBy(id)};
    if (contractPtr)
    {
        return contractPtr->GetContractTagValues();
    }
    throw BringTagValuesError{BringTagValuesError::InvalidPointer{}};
}

CreateResultFileUseCase::CreateResultFileUseCase(fs::path scriptPath, fs::path resultPath) :
    m_scriptPath{scriptPath},
    m_resultPath{resultPath}
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
    const std::string& scriptPath)
{
    std::stringstream command;
    command << "python3 ";
    command << '\'' << scriptPath << '\'' << ' '; ///< script path
    command << '\'' << jsonConfig << '\'' << ' '; ///< json-string
    command << '\'' << savePath << '\''; ///< path for save
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
    command << DOCS_TAR;
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
    command << "tar -cvf ";
    command << savePath;
    command << DOCS_TAR;
    command << " -C ";
    command << savePath;
    command << ".";
    if (std::system(command.str().c_str()) != 0)
    {
        throw std::runtime_error{"Failed to create tar"};
    }
}

//@ TODO завернуть в strand
static void start_script(const std::string& savePath, const std::string& jsonConfig,
    const std::string& scriptPath)
{
    std::stringstream command; ///< @TODO REMOVE
    make_dir(savePath); ///< создает папку для результатов
    call_script(savePath, jsonConfig, scriptPath); ///< вызывает скрипт
    remove_old_archive(savePath); ///< удаляет старый архив, если он есть
    make_archive(savePath); ///< упаковывает документы в архив
}

std::string CreateResultFileUseCase::CreateFile(const std::string& body) const
{
    try
    {
        std::mt19937_64 randomizer{};
        const std::string folderName(std::to_string(randomizer())); ///< название сгенерированной папки
        const std::string generatedFolderPath(static_cast<std::string>(m_resultPath) + folderName + "/"); ///< полный путь сгенерированной папки
        const std::string path = generatedFolderPath + '/' + DOCS_TAR;
        start_script(generatedFolderPath, body, m_scriptPath); ///< @TODO проверить пути и работу скрипта
        return path;
    }
    catch (const std::exception& e)
    {
        // @TODO отловить ошибки, которые реально могут возникнуть
    }
    throw std::runtime_error("Can't create file");
}

} // namespace app
