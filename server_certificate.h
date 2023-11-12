#ifndef SERVER_CERTIFICATE_H
#define SERVER_CERTIFICATE_H

#include <filesystem>

#include <boost/asio/ssl/context.hpp>

inline void load_server_certificate(boost::asio::ssl::context& ctx, std::filesystem::path certsPath)
{
    ctx.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2
        | boost::asio::ssl::context::single_dh_use);
    ctx.use_certificate_chain_file(certsPath.string() + "/user.crt");
    ctx.use_private_key_file(certsPath.string() + "/user.key", boost::asio::ssl::context::pem);
    ctx.use_tmp_dh_file(certsPath.string() + "/dh2048.pem");

    ctx.set_password_callback(
        [](std::size_t,
            boost::asio::ssl::context_base::password_purpose)
        {
            return "";
        }
    );
}

#endif
