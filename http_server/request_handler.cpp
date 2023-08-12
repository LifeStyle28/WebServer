#include "request_handler.h"
#include "response_builder.h"

#include <boost/format.hpp>

namespace http_handler
{

using namespace std::literals;
using namespace app;
namespace sys = boost::system;
namespace fs = std::filesystem;

struct FileEndpoint
{
    FileEndpoint() = delete;
    static constexpr std::string_view FILE_REQ = "/file/getfile"sv;
};

static std::string_view get_mime_type(std::string_view path)
{
    using beast::iequals;
    const auto ext = [&path]
    {
        auto const pos = path.rfind('.');
        if (pos == std::string_view::npos)
        {
            return std::string_view{};
        }
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"sv))
    {
        return ContentType::TEXT_HTML;
    }
    if (iequals(ext, ".html"sv))
    {
        return ContentType::TEXT_HTML;
    }
    if (iequals(ext, ".css"sv))
    {
        return ContentType::TEXT_CSS;
    }
    if (iequals(ext, ".txt"sv))
    {
        return ContentType::TEXT_PLAIN;
    }
    if (iequals(ext, ".js"sv))
    {
        return ContentType::TEXT_JS;
    }
    if (iequals(ext, ".json"sv))
    {
        return ContentType::APP_JSON;
    }
    if (iequals(ext, ".xml"sv))
    {
        return ContentType::APP_XML;
    }
    if (iequals(ext, ".png"sv))
    {
        return ContentType::IMG_PNG;
    }
    if (iequals(ext, ".jpe"sv))
    {
        return ContentType::IMG_JPG;
    }
    if (iequals(ext, ".jpeg"sv))
    {
        return ContentType::IMG_JPG;
    }
    if (iequals(ext, ".jpg"sv))
    {
        return ContentType::IMG_JPG;
    }
    if (iequals(ext, ".gif"sv))
    {
        return ContentType::IMG_GIF;
    }
    if (iequals(ext, ".bmp"sv))
    {
        return ContentType::IMG_BMP;
    }
    if (iequals(ext, ".ico"sv))
    {
        return ContentType::IMG_ICO;
    }
    if (iequals(ext, ".tiff"sv))
    {
        return ContentType::IMG_TIF;
    }
    if (iequals(ext, ".tif"sv))
    {
        return ContentType::IMG_TIF;
    }
    if (iequals(ext, ".svg"sv))
    {
        return ContentType::IMG_SVG;
    }
    if (iequals(ext, ".svgz"sv))
    {
        return ContentType::IMG_SVG;
    }
    if (iequals(ext, ".mp3"sv))
    {
        return ContentType::MP3;
    }
    if (iequals(ext, ".doc"sv))
    {
        return ContentType::DOC;
    }
    if (iequals(ext, ".docx"sv))
    {
        return ContentType::DOCX;
    }
    return ContentType::OCT_STREAM;
}

RequestHandler::RequestHandler(app::Application& app, fs::path configJsonPath,
    fs::path scriptPath, std::filesystem::path resultPath, Strand handlerStrand) :
        m_apiHandler{app},
        m_configJsonPath{configJsonPath},
        m_scriptPath{scriptPath},
        m_resultPath{resultPath},
        m_strand{handlerStrand}
{
}

//@ TODO завернуть в strand и сделать через m_resultPath и m_scriptPath
static void start_script(const std::string& savePath, const std::string& jsonConfig, const std::string& scriptPath)
{
    std::stringstream command;
    command << "python3 ";
    command << '\'' << scriptPath << '\'' << ' '; ///< script path
    command << '\'' << jsonConfig << '\'' << ' '; ///< json-string
    command << '\'' << savePath << '\''; ///< path for save
    std::system(command.str().c_str());

    command.str("");
    command << "tar -cvf ";
    command << savePath;
    command << "docs.tar ";
    command << savePath;
    std::system(command.str().c_str());
}

RequestHandler::FileRequestResult RequestHandler::HandleFileRequest(const StringRequest& req) const
{
    ResponseBuilder builder{req.version(), req.keep_alive()};

    if (req.target() != FileEndpoint::FILE_REQ)
    {
        return builder.MakeBadRequestError("Invalid URL");
    }

    const auto method = req.method();
    if (method != http::verb::head && method != http::verb::get)
    {
        return builder.MakeBadRequestError("Invalid method"sv);
    }

    const std::string path = static_cast<std::string>(m_resultPath) + "docs.tar"; // @TODO сделать уникализацию имени std::mt19937_64
    start_script(m_resultPath, req.body(), m_scriptPath);
    http::file_body::value_type body;
    beast::error_code ec;
    body.open(path.c_str(), beast::file_mode::read, ec);

    if (ec)
    {
        return builder.FromErrorCode(ec);
    }

    const auto size{body.size()};
    auto with_file_headers = [&path, size, &req](auto&& response)
    {
        response.set(http::field::content_type, get_mime_type(path));
        response.content_length(size);
        response.keep_alive(req.keep_alive());
        return std::move(response);
    };

    return with_file_headers(FileResponse{std::piecewise_construct,
                                          std::make_tuple(std::move(body)),
                                          std::make_tuple(http::status::ok, req.version())});
}

StringResponse RequestHandler::ReportServerError(const size_t version, const bool keepAlive) const
{
    ResponseBuilder builder{version, keepAlive};

    return builder.MakeInternalServerError("Internal server error"sv);
}

} // namespace http_handler
