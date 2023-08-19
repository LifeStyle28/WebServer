#include "use_cases.h"

#include <random>

#pragma GCC diagnostic ignored "-Wunused-result" // @TODO - ugly

namespace app
{

namespace fs = std::filesystem;

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

//@ TODO завернуть в strand и сделать через m_resultPath и m_scriptPath
//@ TODO разделить на несколько функций/методов
static void start_script(const std::string& savePath, const std::string& jsonConfig,
    const std::string& scriptPath)
{
    /// создание папки для результатов
    std::stringstream command;
    command << "mkdir -p ";
    command << savePath;
    std::system(command.str().c_str()); // @TODO unused result?

    /// вызов скрипта
    command.str("");
    command << "python3 ";
    command << '\'' << scriptPath << '\'' << ' '; ///< script path
    command << '\'' << jsonConfig << '\'' << ' '; ///< json-string
    command << '\'' << savePath << '\''; ///< path for save
    std::system(command.str().c_str());

    /// упаковка документов в архив
    command.str("");
    command << "tar -cvf ";
    command << savePath;
    command << "docs.tar "; // @TODO - сделать алиас, повторяется больше, чем в 1 месте
    command << savePath;
    std::system(command.str().c_str());
}

std::string CreateResultFileUseCase::CreateFile(const std::string& body) const
{
    try
    {
        std::mt19937_64 randomizer{};
        const std::string folderName(std::to_string(randomizer())); ///< название сгенерированной папки
        const std::string generatedFolderPath(static_cast<std::string>(m_resultPath) + folderName + "/"); ///< полный путь сгенерированной папки
        const std::string path = generatedFolderPath + "/docs.tar";
        start_script(generatedFolderPath, body, m_scriptPath);
        return path;
    }
    catch (const std::exception& e)
    {
        // @TODO отловить ошибки, которые реально могут возникнуть
    }
    throw std::runtime_error("Can't create file");
}

} // namespace app
