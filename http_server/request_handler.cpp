#include "request_handler.h"
#include "response_builder.h"

#include <boost/format.hpp>

namespace http_handler
{

using namespace std::literals;
using namespace app;
namespace sys = boost::system;

RequestHandler::RequestHandler(Application& app, Strand handlerStrand) :
    m_strand{handlerStrand},
    m_apiHandler{app}
{
}

RequestHandler::FileRequestResult RequestHandler::HandleFileRequest(const StringRequest& req) const
{
    ResponseBuilder builder{req.version(), req.keep_alive()};

    const auto method = req.method();
    if (method != http::verb::head && method != http::verb::get)
    {
        return builder.MakeBadRequestError("Invalid method"sv);
    }

    return builder.MakeNotFoundError("Resource not found"sv);
}

StringResponse RequestHandler::ReportServerError(const size_t version, const bool keepAlive) const
{
    ResponseBuilder builder{version, keepAlive};

    return builder.MakeInternalServerError("Internal server error"sv);
}

} // namespace http_handler
