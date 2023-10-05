#include "http_server.h"
#include "logging_request_handler.h"
#include "boost_logger.h"
#include "json_loader.h"

#include <iostream>
#include <thread>
#include <fstream>
#include <optional>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

using namespace std::literals;
using namespace boost_logger;
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace sys = boost::system;
namespace logging = boost::log;
namespace fs = std::filesystem;

namespace
{

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
static void run_workers(unsigned numThreads, const Fn& fn)
{
    numThreads = std::max(1u, numThreads);
    std::vector<std::jthread> workers;
    workers.reserve(numThreads - 1);
    // Запускаем (numThreads - 1) рабочих потоков, выполняющих функцию fn
    while (--numThreads)
    {
        workers.emplace_back(fn);
    }
    fn();
}

struct AppParams
{
    fs::path m_resultPath;
    fs::path m_configJsonPath;
    fs::path m_scriptPath;
    std::optional<fs::path> m_webPath;
};

class InvalidCommandLine : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};

static std::optional<AppParams> parse_command_line(int argc, const char* argv[])
{
    namespace po = boost::program_options;

    po::options_description desc{"Allowed options"};
    std::string resultPath, configJsonPath, scriptPath, webPath;
    desc.add_options()
        ("help,h", "produce help message")
        ("result-path,r", po::value<std::string>(&resultPath)->value_name("dir"s),
            "set result path")
        ("config-file,c", po::value<std::string>(&configJsonPath)->value_name("file"s),
            "set config file path")
        ("script-path,s", po::value<std::string>(&scriptPath)->value_name("file"s),
            "set script file path")
        ("web-path,w", po::value<std::string>(&webPath)->value_name("dir"s),
            "set web files path")
    ;

    po::positional_options_description p;
    p.add("result-path", 1).add("config-file", 1).add("web-path", 1);

    po::variables_map vm;
    po::store(po::command_line_parser{argc, argv}.options(desc).positional(p).run(), vm);
    po::notify(vm);

    AppParams params;
    if (vm.contains("help"s))
    {
        std::cout << desc << std::endl;
        return std::nullopt;
    }
    if (vm.contains("result-path"s))
    {
        params.m_resultPath = resultPath;
    }
    else
    {
        throw InvalidCommandLine{"--result-path is not specified"};
    }
    if (vm.contains("config-file"s))
    {
        params.m_configJsonPath = configJsonPath;
    }
    else
    {
        throw InvalidCommandLine{"--config-file is not specified"};
    }
    if (vm.contains("script-path"s))
    {
        params.m_scriptPath = scriptPath;
    }
    else
    {
        throw InvalidCommandLine{"--script-path is not specified"};
    }
    if (vm.contains("web-path"s))
    {
        params.m_webPath = webPath;
    }

    return params;
}

} // anonymous namespace

int main(int argc, const char* argv[])
{
    InitLogging();
    try
    {
        auto args = parse_command_line(argc, argv);
        if (!args)
        {
            return EXIT_SUCCESS;
        }

        const unsigned numThreads = std::thread::hardware_concurrency();

        // 1. Инициализируем io_context
        net::io_context ioc(numThreads);

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
        model::Config config{json_loader::load_config(args->m_configJsonPath)};
        std::filesystem::path scriptPath{args->m_scriptPath};
        std::filesystem::path resultPath{args->m_resultPath};
        app::Application app{config, scriptPath, resultPath};

        // 4. Создаём обработчик HTTP-запросов
        const auto webPath = (args->m_webPath) ? *args->m_webPath : "";
        auto handlerStrand = net::make_strand(ioc);

        auto handler = std::make_shared<http_handler::RequestHandler>(app, webPath, handlerStrand);
        logging_handler::LoggingRequestHandler<http_handler::RequestHandler> log_handler{*handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const std::string adressString = "0.0.0.0";
        const auto address = net::ip::make_address(adressString);
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port},
            [&log_handler](tcp::endpoint endpoint, auto&& req, auto&& send)
            {
                log_handler(endpoint, std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            }
        );

        json::value customData{{"port"s, port}, {"address"s, adressString}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, customData)
                                << "Server has started"sv;

        // 6. Запускаем обработку асинхронных операций
        run_workers(std::max(1u, numThreads), [&ioc]
        {
            ioc.run();
        });
    }
    catch (const std::exception& ex)
    {
        json::value customData{{"exception"s, ex.what()}, {"code"s, EXIT_FAILURE}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, customData)
                                << "server exited"sv;
        return EXIT_FAILURE;
    }

    json::value customData{{"code"s, 0}};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, customData)
                            << "server exited"sv;
    return 0;
}
