#pragma once

#include "http_server.h"
#include "api_handler.h"

#include <filesystem>
#include <variant>
#include <cassert>

namespace http_handler
{

class RequestHandler : public std::enable_shared_from_this<RequestHandler>
{
    using Strand = net::strand<net::io_context::executor_type>;
public:
    RequestHandler(app::Application& app, std::filesystem::path webPath, Strand handlerStrand);
    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send)
    {
        const auto version = req.version();
        const auto keepAlive = req.keep_alive();

        try
        {
            if (m_apiHandler.IsApiRequest(req) || m_apiHandler.IsOptionsRequest(req))
            {
                auto handle =
                    [self = shared_from_this(), send,
                        req = std::forward<decltype(req)>(req), version, keepAlive]
                    {
                        try
                        {
                            // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                            assert(self->m_strand.running_in_this_thread());
                            // Обрабатываем запрос к API
                            return send(self->m_apiHandler.HandleApiRequest(req));
                        }
                        catch (...)
                        {
                            send(self->ReportServerError(version, keepAlive));
                        }
                    };
                return net::dispatch(m_strand, handle);
            }
            // принимаем response произвольного типа для раскрытия варианта
            return std::visit(
                [&send](auto&& response)
                {
                    send(std::forward<decltype(response)>(response));
                },
                HandleFileRequest(req)
            );
        }
        catch (...)
        {
            send(ReportServerError(version, keepAlive));
        }

    }
private:
    using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;

    FileRequestResult HandleFileRequest(const StringRequest& req) const;
    StringResponse ReportServerError(const size_t version, const bool keepAlive) const;

    ApiHandler m_apiHandler;
    std::filesystem::path m_webPath;
    Strand m_strand;
};

}  // namespace http_handler
