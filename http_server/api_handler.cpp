#include "api_handler.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>

#include "response_builder.h"

namespace http_handler
{

using namespace std::literals;
namespace json = boost::json;

namespace
{

using namespace std::literals;

struct Endpoint
{
    static constexpr std::string_view API_PREFIX = "/api/"sv;
    static constexpr std::string_view FIELDS = "/api/v1/prog/fields"sv;
    static constexpr std::string_view FILE = "/api/v1/prog/file"sv;
};

struct CacheControl
{
    CacheControl() = delete;
    constexpr static std::string_view NO_CACHE = "no-cache"sv;
};

std::string AsString(const json::value& val)
{
    return std::string(val.as_string());
}

struct ErrorKey
{
    ErrorKey() = delete;
    constexpr static boost::string_view CODE_KEY{"code"};
    constexpr static boost::string_view MESSAGE_KEY{"message"};
};

std::string MakeErrorJSON(std::string code, std::string message)
{
    return json::serialize(json::object{{ErrorKey::CODE_KEY, std::move(code)},
        {ErrorKey::MESSAGE_KEY, std::move(message)}});
}

struct FieldsRequest
{
    std::string contractType;
    std::string currencyType;
    std::string currencyKind;

    constexpr static json::string_view CONTRACT_TYPE = "contractType";
    constexpr static json::string_view CURRENCY_TYPE = "currencyType";
    constexpr static json::string_view CURRENCY_KIND = "currencyKind";
};

struct FieldsResponse
{
    FieldsResponse() = delete;
    constexpr static json::string_view FIELD_KEY = "fieldKey";
    constexpr static json::string_view FIELD_VALUE = "fieldValue";
    constexpr static json::string_view TAG = "tag";
};

struct BadRequestErrorCode
{
    BadRequestErrorCode() = delete;
    const inline static std::string INVALID_ARGUMENT = "invalidArgument"s;
};

struct MethodNotAllowedErrorCode
{
    MethodNotAllowedErrorCode() = delete;
    const inline static std::string INVALID_METHOD = "invalidMethod"s;
};

class APIError : public std::runtime_error
{
public:
    APIError(http::status status, std::string code, const std::string& msg) :
        runtime_error{msg},
        m_code{std::move(code)},
        m_status{status}
    {
    }

    const std::string& GetCode() const noexcept
    {
        return m_code;
    }

    http::status GetStatus() const noexcept
    {
        return m_status;
    }
private:
    std::string m_code;
    http::status m_status;
};

class BadRequest : public APIError
{
public:
    BadRequest(std::string code, const std::string& msg) :
        APIError{http::status::bad_request, std::move(code), msg}
    {
    }
};

class MethodNotAllowed : public APIError
{
public:
    MethodNotAllowed(std::string code, const std::string& msg, http::verb allowed_method) :
        APIError{http::status::method_not_allowed, std::move(code), msg},
        m_allowedMethods{MakeAllowedMethods(allowed_method)}
    {
    }

    const std::vector<http::verb>& GetAllowedMethods() const noexcept
    {
        return m_allowedMethods;
    }
private:
    static std::vector<http::verb> MakeAllowedMethods(const http::verb method)
    {
        if (method == http::verb::get || method == http::verb::head)
        {
            return {http::verb::get, http::verb::head};
        }
        return {method};
    }

    std::vector<http::verb> m_allowedMethods;
};

class RequestHandlingContext
{
public:
    RequestHandlingContext(const StringRequest& request, app::Application& app) :
        m_request{request},
        m_app{app}
    {
    }

    StringResponse Handle() const
    {
        StringResponse response = HandleImpl();
        response.set(http::field::cache_control, CacheControl::NO_CACHE);
        if (m_request.method() == http::verb::head)
        {
            response.prepare_payload();
            response.body().clear();
        }
        return response;
    }
private:
    StringResponse HandleImpl() const
    {
        try
        {
            const auto target = m_request.target();
            if (target == Endpoint::FIELDS)
            {
                return FieldsReqHandle();
            }
            throw BadRequest("badRequest"s, "Invalid endpoint"s);
        }
        catch (const MethodNotAllowed& e)
        {
            auto response = m_builder.MakeJSONResponse(MakeErrorJSON(e.GetCode(), e.what()), e.GetStatus());
            std::string allow_header;
            for (auto method : e.GetAllowedMethods())
            {
                if (!allow_header.empty())
                {
                    allow_header += ", "sv;
                }
                allow_header += http::to_string(method);
            }
            response.insert(http::field::allow, allow_header);
            return response;
        }
        catch (const APIError& e)
        {
            return m_builder.MakeJSONResponse(MakeErrorJSON(e.GetCode(), e.what()), e.GetStatus());
        }
    }

    StringResponse FieldsReqHandle() const
    {
        EnsureMethod(http::verb::get);

        json::array fieldsArr;
        const auto json = json::serialize(fieldsArr);
        return m_builder.MakeJSONResponse(json);
    }

    void EnsureJsonContentType() const
    {
        if (auto it = m_request.find(http::field::content_type);
            it == m_request.end() || !beast::iequals(it->value(), ContentType::APP_JSON))
        {
            throw BadRequest(BadRequestErrorCode::INVALID_ARGUMENT, "Invalid content type"s);
        }
    }

    void EnsureMethod(http::verb v) const
    {
        // GET method also accepts HEAD
        if (m_request.method() == http::verb::head && v == http::verb::get)
        {
            return;
        }
        if (m_request.method() != v)
        {
            throw MethodNotAllowed{MethodNotAllowedErrorCode::INVALID_METHOD, "Invalid method"s, v};
        }
    }

    const StringRequest& m_request;
    ResponseBuilder m_builder{m_request.version(), m_request.keep_alive()};
    app::Application& m_app;
};

} // namespace

ApiHandler::ApiHandler(app::Application& app) : m_app{app}
{
}

bool ApiHandler::IsApiRequest(const StringRequest& request) const
{
    return request.target().starts_with(Endpoint::API_PREFIX);
}

StringResponse ApiHandler::HandleApiRequest(const StringRequest& request)
{
    RequestHandlingContext ctx(request, m_app);
    return ctx.Handle();
}

} // namespace http_handler
