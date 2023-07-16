#include "http_server.h"
#include "boost_logger.h"

#include <boost/asio/dispatch.hpp>
#include <boost/json.hpp>

namespace http_server
{

using namespace std::literals;
namespace json = boost::json;

void ReportError(beast::error_code ec, std::string where)
{
    json::value custom_data
    {
        {"code"s, ec.value()},
        {"text"s, ec.message()},
        {"where"s, where}
    };
    BOOST_LOG_TRIVIAL(info) << boost::log::add_value(boost_logger::additional_data, custom_data)
                            << "error"sv;
}

SessionBase::SessionBase(tcp::socket&& socket) :
    m_stream(std::move(socket))
{
}

SessionBase::~SessionBase()
{
}

void SessionBase::Run()
{
    // Вызываем метод Read, используя executor объекта m_stream.
    // Таким образом вся работа со m_stream будет выполняться, используя его executor
    net::dispatch(m_stream.get_executor(),
                  beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

tcp::endpoint SessionBase::GetEndpoint() const
{
    return m_stream.socket().remote_endpoint();
}

void SessionBase::Read()
{
    // Очищаем запрос от прежнего значения (метод Read может быть вызван несколько раз)
    m_request = {};
    m_stream.expires_after(30s);
    // Считываем m_request из m_stream, используя m_buffer для хранения считанных данных
    http::async_read(m_stream, m_buffer, m_request,
                     // По окончании операции будет вызван метод OnRead
                     beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
}

void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read)
{
    if (ec == http::error::end_of_stream)
    {
        // Нормальная ситуация - клиент закрыл соединение
        return Close();
    }
    if (ec)
    {
        return ReportError(ec, "read");
    }
    HandleRequest(std::move(m_request));
}

void SessionBase::OnWrite(bool close, beast::error_code ec,
    [[maybe_unused]] std::size_t bytes_written)
{
    if (ec)
    {
        return ReportError(ec, "write");
    }

    if (close)
    {
        // Семантика ответа требует закрыть соединение
        return Close();
    }

    // Считываем следующий запрос
    Read();
}

void SessionBase::Close()
{
    beast::error_code ec;
    m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
}

}  // namespace http_server
