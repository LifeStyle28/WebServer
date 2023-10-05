#include "api_handler.h"
#include "response_builder.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <cstdlib>

namespace http_handler
{

using namespace std::literals;
namespace json = boost::json;

namespace
{

using namespace std::literals;

// @TODO - не нравится дублирование этой функции в разных файлах
static std::string json_val_as_string(const json::value& val)
{
    return std::string(val.as_string());
}

struct Endpoint
{
    Endpoint() = delete;
    static constexpr std::string_view API_PREFIX = "/api/"sv;
    static constexpr std::string_view TAG_VALUES = "/api/v1/prog/tag_values"sv;
    static constexpr std::string_view FILLED_CONTENT = "/api/v1/prog/filled_content"sv;
};

struct CacheControl
{
    CacheControl() = delete;
    static constexpr std::string_view NO_CACHE = "no-cache"sv;
};

struct ErrorKey
{
    ErrorKey() = delete;
    static constexpr boost::string_view CODE_KEY{"code"};
    static constexpr boost::string_view MESSAGE_KEY{"message"};
};

static std::string MakeErrorJSON(std::string code, std::string message)
{
    return json::serialize(json::object{{ErrorKey::CODE_KEY, std::move(code)},
        {ErrorKey::MESSAGE_KEY, std::move(message)}});
}

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

static TagValuesRequest parse_tag_values_request(boost::string_view body)
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

static json::array tag_values_to_json(const model::Contract::ContractTagValues& tagValues)
{
    json::array tagValuesArr;
    for (const auto& tagValue : tagValues)
    {
        json::object obj{{TagValuesResponse::KEY, tagValue.m_key}, {TagValuesResponse::TAG, tagValue.m_tag},
            {TagValuesResponse::VALUE, tagValue.m_value}};
        tagValuesArr.emplace_back(obj);
    }
    return tagValuesArr;
}

struct BringTagValuesErrorReporter
{
    StringResponse operator()(const app::BringTagValuesError::InvalidPointer&) const
    {
        return m_builder.MakeJSONResponse(
            MakeErrorJSON(BadRequestErrorCode::INVALID_ARGUMENT, "Contract not found"s), // @TODO придумать новый тип ошибки?
            http::status::not_found);
    }

    StringResponse operator()(const app::BringTagValuesError::Dummy&) const
    {
        return m_builder.MakeJSONResponse(
            MakeErrorJSON(BadRequestErrorCode::INVALID_ARGUMENT, "Dummy"s),
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
            if (target == Endpoint::TAG_VALUES)
            {
                return TagValuesReqHandle();
            }
            else if (target == Endpoint::FILLED_CONTENT)
            {
                return FilledContentHandle();
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
        EnsureMethod(http::verb::get);
        EnsureJsonContentType();

        try
        {
            const auto tagValuesReq{parse_tag_values_request(m_request.body())};
            const auto& tagValues{m_app.GetTagValues(model::Contract::Id(tagValuesReq.contractType + '_'
                + tagValuesReq.currencyType + '_' + tagValuesReq.currencyKind))};

            m_app.SaveContractDuration(tagValuesReq.contractDuration);

            return m_builder.MakeJSONResponse(json::serialize(tag_values_to_json(tagValues)));
        }
        catch (const app::BringTagValuesError& e)
        {
            return std::visit(BringTagValuesErrorReporter{m_builder}, e.GetReason());
        }
    }

    StringResponse FilledContentHandle() const
    {
        EnsureMethod(http::verb::get);
        EnsureJsonContentType();

        try
        {
            const auto fileName{m_app.GetResultFileName(m_request.body())};
            json::object obj{{FilledContentResponse::FILE_NAME, fileName}};
            return m_builder.MakeJSONResponse(json::serialize(obj));
        }
        catch (const std::runtime_error& e)
        {
            // @TODO !!! - пробросить нормальные исключение, написать нормальные описания для них
            throw BadRequest("badRequest"s, e.what());
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
