#pragma once

#include "model.h"

#include <filesystem>

#include <boost/utility/string_view.hpp>

namespace json_loader
{

struct ConfigToken
{
    ConfigToken() = delete;
    static constexpr boost::string_view PERCENT{"percent"};
    static constexpr boost::string_view EMAIL{"email"};
};

std::string load_file_as_string(const std::filesystem::path& jsonPath);
model::Config load_config(const std::filesystem::path& jsonPath);

} // namespace json_loader
