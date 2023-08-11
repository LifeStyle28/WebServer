#pragma once

#include "model.h"

#include <filesystem>

namespace json_loader
{

model::Config load_config(const std::filesystem::path& jsonPath);

} // namespace json_loader
