#pragma once

#include "http_server.h"

#include <string_view>

namespace http_handler
{

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using namespace std::literals;

using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using EmptyResponse = http::response<http::empty_body>;

struct ContentType
{
    ContentType() = delete;
    static constexpr std::string_view TEXT_HTML = "text/html"sv;
    static constexpr std::string_view APP_JSON = "application/json"sv;
    static constexpr std::string_view TEXT_CSS = "text/css"sv;
    static constexpr std::string_view TEXT_PLAIN = "text/plain"sv;
    static constexpr std::string_view TEXT_JS = "text/javascript"sv;
    static constexpr std::string_view APP_XML = "application/xml"sv;
    static constexpr std::string_view IMG_PNG = "image/png"sv;
    static constexpr std::string_view IMG_JPG = "image/jpeg"sv;
    static constexpr std::string_view IMG_GIF = "image/gif"sv;
    static constexpr std::string_view IMG_BMP = "image/bmp"sv;
    static constexpr std::string_view IMG_ICO = "image/vnd.microsoft.icon"sv;
    static constexpr std::string_view IMG_TIF = "image/tiff"sv;
    static constexpr std::string_view IMG_SVG = "image/svg+xml"sv;
    static constexpr std::string_view MP3 = "audio/mpeg"sv;
    static constexpr std::string_view OCT_STREAM = "application/octet-stream"sv;
    static constexpr std::string_view DOC = "application/msword"sv;
    static constexpr std::string_view DOCX =
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document"sv;
};

struct AccessControl
{
    AccessControl() = delete;
    static constexpr std::string_view ORIGIN = "https://www.rustonn.ru"sv;
    static constexpr std::string_view CREDENTIALS = "true"sv;
    static constexpr std::string_view METHODS = "GET, HEAD, POST, OPTIONS"sv;
    static constexpr std::string_view HEADERS = "Content-Type, Authorization"sv;
};

} // namespace http_handler
