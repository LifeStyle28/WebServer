#pragma once

#include "http_server.h"

#include <string_view>

namespace http_handler
{

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using EmptyResponse = http::response<http::empty_body>;

struct ContentType
{
    ContentType() = delete;
    static constexpr std::string_view TEXT_HTML = "text/html";
    static constexpr std::string_view APP_JSON = "application/json";
    static constexpr std::string_view TEXT_CSS = "text/css";
    static constexpr std::string_view TEXT_PLAIN = "text/plain";
    static constexpr std::string_view TEXT_JS = "text/javascript";
    static constexpr std::string_view APP_XML = "application/xml";
    static constexpr std::string_view IMG_PNG = "image/png";
    static constexpr std::string_view IMG_JPG = "image/jpeg";
    static constexpr std::string_view IMG_GIF = "image/gif";
    static constexpr std::string_view IMG_BMP = "image/bmp";
    static constexpr std::string_view IMG_ICO = "image/vnd.microsoft.icon";
    static constexpr std::string_view IMG_TIF = "image/tiff";
    static constexpr std::string_view IMG_SVG = "image/svg+xml";
    static constexpr std::string_view MP3 = "audio/mpegs";
    static constexpr std::string_view OCT_STREAM = "application/octet-stream";
};

} // namespace http_handler