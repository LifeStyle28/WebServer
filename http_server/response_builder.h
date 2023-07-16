#pragma once

#include "http_types.h"

namespace http_handler
{

class ResponseBuilder
{
public:
    ResponseBuilder(const size_t version, const bool keepAlive);

    StringResponse MakeStringResponse(std::string_view body, std::string_view contentType,
        http::status status = http::status::ok) const;
    StringResponse MakePlainTextResponse(std::string_view body,
        http::status status = http::status::ok) const;
    StringResponse MakeJSONResponse(std::string_view body,
        http::status status = http::status::ok) const;
    StringResponse MakeBadRequestError(std::string_view why) const;
    StringResponse MakeNotFoundError(std::string_view why) const;
    StringResponse MakeInternalServerError(std::string_view why) const;
    StringResponse MakeForbiddenError(std::string_view why) const;
    StringResponse FromErrorCode(beast::error_code ec) const;
private:
    size_t m_version;
    bool m_keepAlive;
};

} // namespace http_handler
