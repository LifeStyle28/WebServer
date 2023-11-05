#include "boost_logger.h"

#include <boost/log/core.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time.hpp>

namespace boost_logger
{

using namespace std::literals;
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm)
{
    const json::value value
    {
        {"timestamp"s, to_iso_extended_string(*rec[timestamp])},
        {"data"s, *rec[additional_data]},
        {"message"s, *rec[expr::smessage]}
    };
    strm << json::serialize(value);
}

void InitLogging()
{
    logging::add_common_attributes();
    logging::add_console_log(std::cout, keywords::format = &MyFormatter,
        logging::keywords::auto_flush = true);
    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info);
}

} // namespace boost_logger
