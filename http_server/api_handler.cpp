#include "api_handler.h"
#include "response_builder.h"

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>

namespace http_handler
{

namespace
{

namespace json = boost::json;
using namespace std::literals;

// @TODO - не нравится дублирование этой функции в разных файлах
std::string json_val_as_string(const json::value& val)
{
    return std::string(val.as_string());
}

struct Endpoint
{
    Endpoint() = delete;
    static constexpr std::string_view API_PREFIX = "/api/"sv;
    static constexpr std::string_view TAG_VALUES = "/api/v1/prog/tag_values"sv;
    static constexpr std::string_view FILLED_CONTENT = "/api/v1/prog/filled_content"sv;
    static constexpr std::string_view CHANGE_PERCENT = "/api/v1/prog/change_percent"sv;
};

struct CacheControl
{
    CacheControl() = delete;
    static constexpr std::string_view NO_CACHE = "no-cache"sv;
};

struct AccessControl
{
    AccessControl() = delete;
    static constexpr std::string_view ORIGIN = "https://www.rustonn.ru"sv;
    static constexpr std::string_view CREDENTIALS = "true"sv;
    static constexpr std::string_view METHODS = "GET, HEAD, POST, OPTIONS"sv;
    static constexpr std::string_view HEADERS = "Content-Type, Authorization"sv;
};

struct ErrorKey
{
    ErrorKey() = delete;
    static constexpr boost::string_view CODE_KEY{"code"};
    static constexpr boost::string_view MESSAGE_KEY{"message"};
};

std::string MakeErrorJSON(std::string code, std::string message)
{
    return json::serialize(json::object{{ErrorKey::CODE_KEY, std::move(code)},
        {ErrorKey::MESSAGE_KEY, std::move(message)}});
}

struct ChangePercentRequest
{
    size_t percent;

    static constexpr json::string_view PERCENT = "percent";
};

struct TagValuesRequest
{
    std::string contractType;
    std::string currencyType;
    std::string currencyKind;
    size_t contractDuration;

    static constexpr json::string_view CONTRACT_TYPE = "contractType";
    static constexpr json::string_view CURRENCY_TYPE = "currencyType";
    static constexpr json::string_view CURRENCY_KIND = "currencyKind";
    static constexpr json::string_view CONTRACT_DURATION = "contractDuration";
};

struct TagValuesResponse
{
    TagValuesResponse() = delete;
    static constexpr json::string_view TOKEN = "token";
    static constexpr json::string_view TAG_VALUES = "tag_values";
    static constexpr json::string_view KEY = "key";
    static constexpr json::string_view TAG = "tag";
    static constexpr json::string_view VALUE = "value";
};

struct FilledContentResponse
{
    FilledContentResponse() = delete;
    static constexpr json::string_view FILE_NAME = "fileName";
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

struct UnauthorizedErrorCode
{
    UnauthorizedErrorCode() = delete;
    const inline static std::string INVALID_TOKEN = "invalidToken"s;
    const inline static std::string UNKNOWN_TOKEN = "unknownToken"s;
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

class UnauthorizedError : public APIError
{
public:
    UnauthorizedError(std::string code, const std::string& msg) :
        APIError(http::status::unauthorized, std::move(code), msg)
    {
    }
};

TagValuesRequest parse_tag_values_request(boost::string_view body)
{
    try
    {
        const auto reqJson = json::parse(body);
        const auto& obj = reqJson.as_object();

        return
        {
            json_val_as_string(obj.at(TagValuesRequest::CONTRACT_TYPE)),
            json_val_as_string(obj.at(TagValuesRequest::CURRENCY_TYPE)),
            json_val_as_string(obj.at(TagValuesRequest::CURRENCY_KIND)),
            static_cast<size_t>(obj.at(TagValuesRequest::CONTRACT_DURATION).as_int64())
        };
    }
    catch (const std::out_of_range& e)
    {
        throw BadRequest(BadRequestErrorCode::INVALID_ARGUMENT,
            "Tag values request parse error: "s + e.what());
    }
    catch (const boost::system::system_error& e)
    {
        throw BadRequest(BadRequestErrorCode::INVALID_ARGUMENT,
            "Tag values request parse error: "s + e.what());
    }
}

json::object create_conn_result_to_json(app::CreateConnectionResult result)
{
    json::array tagValuesArr;
    for (const auto& tagValue : result.m_tagValues)
    {
        json::object obj{{TagValuesResponse::KEY, tagValue.m_key}, {TagValuesResponse::TAG, tagValue.m_tag},
            {TagValuesResponse::VALUE, tagValue.m_value}};
        tagValuesArr.emplace_back(obj);
    }
    return
    {
        {TagValuesResponse::TOKEN, *result.m_token},
        {TagValuesResponse::TAG_VALUES, std::move(tagValuesArr)}
    };
}

size_t parse_change_percent_request(boost::string_view body)
{
    try
    {
        const auto reqJson = json::parse(body);
        const auto& obj = reqJson.as_object();

        return obj.at(ChangePercentRequest::PERCENT).as_int64();
    }
    catch (const std::out_of_range& e)
    {
        throw BadRequest(BadRequestErrorCode::INVALID_ARGUMENT,
            "Percent request parse error: "s + e.what());
    }
    catch (const boost::system::system_error& e)
    {
        throw BadRequest(BadRequestErrorCode::INVALID_ARGUMENT,
            "Percent request parse error: "s + e.what());
    }
}

struct CreateConnectionErrorReporter
{
    StringResponse operator()(const app::CreateConnectionError::InvalidPointer&) const
    {
        return m_builder.MakeJSONResponse(
            MakeErrorJSON(BadRequestErrorCode::INVALID_ARGUMENT, "Contract not found"s), // @TODO придумать новый тип ошибки?
            http::status::not_found);
    }

    StringResponse operator()(const app::CreateConnectionError::OtherReason&) const
    {
        return m_builder.MakeJSONResponse(
            MakeErrorJSON(BadRequestErrorCode::INVALID_ARGUMENT, "Other reason with create conn"s),
            http::status::bad_request);
    }

    const ResponseBuilder& m_builder;
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
        response.set(http::field::access_control_allow_origin, AccessControl::ORIGIN);
        response.set(http::field::access_control_allow_credentials, AccessControl::CREDENTIALS);
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
            if (m_request.method() == http::verb::options)
            {
                auto response = m_builder.MakePlainTextResponse("");
                response.set(http::field::access_control_allow_methods, AccessControl::METHODS);
                response.set(http::field::access_control_allow_headers, AccessControl::HEADERS);
                return response;
            }

            const auto target = m_request.target();
            if (target == Endpoint::TAG_VALUES)
            {
                return TagValuesReqHandle();
            }
            else if (target == Endpoint::FILLED_CONTENT)
            {
                return FilledContentHandle();
            }
            else if (target == Endpoint::CHANGE_PERCENT)
            {
                return ChangePercentHandle();
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

    StringResponse TagValuesReqHandle() const
    {
        EnsureMethod(http::verb::post);
        EnsureJsonContentType();

        try
        {
            const auto tagValuesReq{parse_tag_values_request(m_request.body())};
            const auto createConnResult{m_app.CreateConnection(model::Contract::Id(
                tagValuesReq.contractType + '_' +
                tagValuesReq.currencyType + '_' +
                tagValuesReq.currencyKind), tagValuesReq.contractDuration)
            };

            return m_builder.MakeJSONResponse(
                json::serialize(create_conn_result_to_json(std::move(createConnResult)))
            );
        }
        catch (const app::CreateConnectionError& e)
        {
            return std::visit(CreateConnectionErrorReporter{m_builder}, e.GetReason());
        }
    }

    StringResponse FilledContentHandle() const
    {
        EnsureMethod(http::verb::post);
        EnsureJsonContentType();

        return ExecuteAuthorized([this](const app::Token& token)
        {
            const auto fileName{m_app.GetResultFileName(m_request.body(), token)};
            json::object obj{{FilledContentResponse::FILE_NAME, fileName}};
            return m_builder.MakeJSONResponse(json::serialize(obj));
        });
    }

    StringResponse ChangePercentHandle() const
    {
        EnsureMethod(http::verb::post);
        EnsureJsonContentType();

        try
        {
            const auto percent{parse_change_percent_request(m_request.body())};
            m_app.ChangePercent(percent);

            return m_builder.MakePlainTextResponse("Change Percent is successful"sv);
        }
        catch (const app::ChangePercentError& e)
        {
            return m_builder.MakeInternalServerError(e.what());
        }
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

    template <typename Fn>
    StringResponse ExecuteAuthorized(Fn&& fn) const
    {
        try
        {
            return fn(ExtractTokenFromRequest());
        }
        catch (const std::exception& e)
        {
            throw UnauthorizedError{UnauthorizedErrorCode::UNKNOWN_TOKEN, e.what()};
        }
    }

    app::Token ExtractTokenFromRequest() const
    {
        const auto it = m_request.find(http::field::authorization);
        const auto errCode = UnauthorizedErrorCode::INVALID_TOKEN;
        if (it == m_request.end())
        {
            throw UnauthorizedError{errCode, "Authorization header is required"s};
        }

        auto value = it->value();
        constexpr auto TOKEN_START = "bearer "sv;
        if (!boost::iequals(value.substr(0, TOKEN_START.size()), TOKEN_START))
        {
            throw UnauthorizedError(errCode, "Invalid Authorization header"s);
        }

        const auto token = value.substr(TOKEN_START.size());
        const auto tokenStartPos = token.find_first_not_of(' ');
        if (tokenStartPos == token.npos)
        {
            throw UnauthorizedError(errCode, "Invalid bearer token"s);
        }

        auto tokenEndPos = token.find(' ', tokenStartPos);
        if (tokenEndPos == token.npos)
        {
            tokenEndPos = token.size();
        }
        std::string tokenStr = {&token[tokenStartPos], tokenEndPos - tokenStartPos};
        constexpr size_t TOKEN_LENGTH = 32;
        if (tokenStr.size() != TOKEN_LENGTH ||
            boost::algorithm::any_of(tokenStr, [](unsigned char ch)
            {
                   return !std::isxdigit(ch);
            }))
        {
            throw UnauthorizedError(errCode, "Token must contain exactly 32 hex digits"s);
        }
        return app::Token{std::move(tokenStr)};
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

bool ApiHandler::IsOptionsRequest(const StringRequest& request) const
{
    return request.method() == http::verb::options;
}

StringResponse ApiHandler::HandleApiRequest(const StringRequest& request)
{
    RequestHandlingContext ctx(request, m_app);
    return ctx.Handle();
}

} // namespace http_handler
