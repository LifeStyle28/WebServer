#include "http_server.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
// #include <boost/program_options.hpp>
#include <iostream>
#include <thread>

#include "logging_request_handler.h"
#include "boost_logger.h"

using namespace std::literals;
using namespace boost_logger;
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace sys = boost::system;
namespace logging = boost::log;
namespace fs = std::filesystem;
// namespace po = boost::program_options;

namespace
{

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned num_threads, const Fn& fn)
{
    num_threads = std::max(1u, num_threads);
    std::vector<std::jthread> workers;
    workers.reserve(num_threads - 1);
    // Запускаем (num_threads - 1) рабочих потоков, выполняющих функцию fn
    while (--num_threads)
    {
        workers.emplace_back(fn);
    }
    fn();
}

} // anonymous namespace

int main(int argc, const char* argv[])
{
    InitLogging();
    try
    {
        const unsigned num_threads = std::thread::hardware_concurrency();

        // 1. Инициализируем io_context
        net::io_context ioc(num_threads);

        // 2. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number)
        {
            if (!ec)
            {
                ioc.stop();
            }
        });

        // 3. Создаем экземпляр приложения
        model::Config config; // @TODO - сделать парсинг конфига, создать config.json
        app::Application app{config};

        // 4. Создаём обработчик HTTP-запросов
        auto handler_strand = net::make_strand(ioc);

        auto handler = std::make_shared<http_handler::RequestHandler>(app, handler_strand);
        logging_handler::LoggingRequestHandler<http_handler::RequestHandler> log_handler{*handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const std::string str_address = "0.0.0.0";
        const auto address = net::ip::make_address(str_address);
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port},
            [&log_handler](tcp::endpoint endpoint, auto&& req, auto&& send)
            {
                log_handler(endpoint, std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            }
        );

        json::value custom_data{{"port"s, port}, {"address"s, str_address}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                                << "Server has started"sv;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc]
        {
            ioc.run();
        });
    }
    catch (const std::exception& ex)
    {
        json::value custom_data{{"exception"s, ex.what()}, {"code"s, EXIT_FAILURE}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                                << "server exited"sv;
        return EXIT_FAILURE;
    }

    json::value custom_data{{"code"s, 0}};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data)
                            << "server exited"sv;
    return 0;
}
