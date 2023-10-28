#include "request_handler.h"
#include "response_builder.h"

#include <charconv>
#include <optional>

#include <boost/format.hpp>

namespace http_handler
{

using namespace std::literals;
using namespace app;
namespace sys = boost::system;
namespace fs = std::filesystem;

constexpr std::string_view FIRST_PAGE_HTML = "first_page.html"sv;

struct FileEndpoint
{
    FileEndpoint() = delete;
    static constexpr std::string_view FILE_REQ = "/file/getfile"sv;
};

static std::string_view get_mime_type(std::string_view path)
{
    using beast::iequals;
    const auto ext = [&path] {
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

RequestHandler::RequestHandler(app::Application& app, fs::path webPath, Strand handlerStrand) :
    m_apiHandler{app}, m_webPath{webPath}, m_strand{handlerStrand}
{
}

static std::optional<std::string> decode_path(std::string_view target)
{
    std::string result;
    result.reserve(target.size());

    std::string_view::iterator it = target.begin();
    while (it != target.end())
    {
        if (*it == '+')
        {
            result.push_back(' ');
        }
        else if (*it == '%')
        {
            if (std::next(it) == target.end() || std::next(std::next(it)) == target.end())
            {
                return std::nullopt;
            }
            if (char value; std::from_chars(it + 1, it + 2, value, 16).ec == std::errc{})
            {
                result.push_back(value);
            }
            else
            {
                return std::nullopt;
            }
            it += 2;
        }
        else
        {
            result.push_back(*it);
        }
        ++it;
    }

    return result;
}

static bool is_sub_path(fs::path path, fs::path base)
{
    // Приводим оба пути к каноничному виду (без . и ..)
    path = fs::weakly_canonical(path);
    base = fs::weakly_canonical(base);

    // Проверяем, что все компоненты base содержатся внутри path
    for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p)
    {
        if (p == path.end() || *p != *b)
        {
            return false;
        }
    }
    return true;
}

static std::optional<fs::path> make_absolute_path(fs::path base, std::string_view path)
{
    try
    {
        fs::path fs_path{path.begin(), path.end()};
        fs_path = fs_path.lexically_relative("/");
        auto abs_path = base / fs_path;
        if (fs::is_directory(abs_path))
        {
            abs_path /= FIRST_PAGE_HTML;
            abs_path = fs::weakly_canonical(abs_path);
        }

        return abs_path;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

RequestHandler::FileRequestResult RequestHandler::HandleFileRequest(const StringRequest& req) const
{
    ResponseBuilder builder{req.version(), req.keep_alive()};

    const auto method = req.method();
    if (method != http::verb::head && method != http::verb::get)
    {
        return builder.MakeBadRequestError("Invalid method"sv);
    }

    const auto target = decode_path(req.target());
    if (!target)
    {
        return builder.MakeBadRequestError("Invalid URL");
    }
    assert(!target->empty() && target->front() == '/');

    if (auto opt = make_absolute_path(m_webPath, *target))
    {
        if (!is_sub_path(*opt, m_webPath))
        {
            return builder.MakeBadRequestError("Invalid URL");
        }

        const auto pathStr = opt->string();
        http::file_body::value_type body;
        beast::error_code ec;
        body.open(pathStr.c_str(), beast::file_mode::read, ec);
        if (ec)
        {
            return builder.FromErrorCode(ec);
        }

        const auto size = body.size();

        auto with_file_headers = [&pathStr, size, &req](auto&& response)
        {
            response.set(http::field::content_type, get_mime_type(pathStr));
            response.content_length(size);
            response.keep_alive(req.keep_alive());
            return std::move(response);
        };

        if (method == http::verb::head)
        {
            return with_file_headers(EmptyResponse{http::status::ok, req.version()});
        }

        return with_file_headers(FileResponse{std::piecewise_construct,
            std::make_tuple(std::move(body)), std::make_tuple(http::status::ok, req.version())});
    }
    else
    {
        return builder.MakeNotFoundError(
            (boost::format("Resource %1% not found"s) % *target).str());
    }
}

StringResponse RequestHandler::ReportServerError(const size_t version, const bool keepAlive) const
{
    ResponseBuilder builder{version, keepAlive};

    return builder.MakeInternalServerError("Internal server error"sv);
}

} // namespace http_handler
