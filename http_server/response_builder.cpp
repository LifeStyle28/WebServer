#include "response_builder.h"

namespace http_handler
{

using namespace std::literals;

ResponseBuilder::ResponseBuilder(const size_t version, const bool keepAlive) :
    m_version{version},
    m_keepAlive{keepAlive}
{
}

StringResponse ResponseBuilder::MakeStringResponse(std::string_view body,
    std::string_view contentType, http::status status) const
{
    StringResponse response(status, m_version);
    response.set(http::field::content_type, contentType);
    response.body() = body;
    response.keep_alive(m_keepAlive);
    response.prepare_payload();
    return response;
}

StringResponse ResponseBuilder::MakePlainTextResponse(std::string_view body,
    http::status status) const
{
    return MakeStringResponse(body, ContentType::TEXT_PLAIN, status);
}

StringResponse ResponseBuilder::MakeJSONResponse(std::string_view body, http::status status) const
{
    return MakeStringResponse(body, ContentType::APP_JSON, status);
}

StringResponse ResponseBuilder::MakeBadRequestError(std::string_view why) const
{
    return MakePlainTextResponse(why, http::status::bad_request);
}

StringResponse ResponseBuilder::MakeNotFoundError(std::string_view why) const
{
    return MakePlainTextResponse(why, http::status::not_found);
}

StringResponse ResponseBuilder::MakeInternalServerError(std::string_view why) const
{
    return MakePlainTextResponse(why, http::status::internal_server_error);
}

StringResponse ResponseBuilder::MakeForbiddenError(std::string_view why) const
{
    return MakePlainTextResponse(why, http::status::forbidden);
}

StringResponse ResponseBuilder::FromErrorCode(beast::error_code ec) const
{
    if (ec == beast::errc::no_such_file_or_directory)
    {
        return MakeNotFoundError("File not found"sv);
    }
    if (ec == beast::errc::permission_denied)
    {
        return MakeForbiddenError("Access denied"sv);
    }
    if (ec)
    {
        return MakeInternalServerError("Internal server error"sv);
    }
    return MakePlainTextResponse({});
}

} // namespace http_handler
