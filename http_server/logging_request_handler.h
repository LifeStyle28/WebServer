#pragma once

#include "request_handler.h"
#include "../logger/boost_logger.h"

#include <chrono>

namespace logging_handler
{

namespace ph = std::placeholders;
namespace json = boost::json;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

template<class SomeRequestHandler>
class LoggingRequestHandler
{
    static void LogRequest(tcp::endpoint& endpoint, auto&& req)
    {
        using namespace std::literals;
        json::value custom_data
        {
            {"ip"s, endpoint.address().to_string()},
            {"URI"s, req.target()},
            {"method"s, req.method_string()}
        };
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(boost_logger::additional_data, custom_data)
                                << "request received"sv;
    }
    static void LogResponse(tcp::endpoint& endpoint, auto&& response, auto&& duration)
    {
        using namespace std::literals;
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        auto get_content_type = [&response]() -> std::string
        {
            if (response.count(http::field::content_type) == 0)
            {
                return "";
            }
            return std::string(response[http::field::content_type]);
        };

        json::value custom_data
        {
            {"ip"s, endpoint.address().to_string()},
            {"response_time"s, ms},
            {"code"s, response.result_int()},
            {"content_type"s, get_content_type()}
        };
        BOOST_LOG_TRIVIAL(info) << boost::log::add_value(boost_logger::additional_data, custom_data)
                                << "response sent"sv;
    }
public:
    LoggingRequestHandler(SomeRequestHandler& handler) :
        m_decorated{handler}
    {
    }
    LoggingRequestHandler(const LoggingRequestHandler&) = delete;
    LoggingRequestHandler& operator=(const LoggingRequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(tcp::endpoint endpoint,
        http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
    {
        LogRequest(endpoint, req);
        const auto start = std::chrono::steady_clock::now();
        auto log_response = [&endpoint, send = std::forward<Send>(send), start] (auto&& response)
        {
            const auto end = std::chrono::steady_clock::now();
            LogResponse(endpoint, response, end - start);
            send(std::forward<decltype(response)>(response));
        };
        m_decorated(std::forward<decltype(req)>(req), log_response);
    }

private:
    SomeRequestHandler& m_decorated;
};

} // namespace logging_handler
