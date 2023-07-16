#pragma once

#include <vector>
#include <memory>

namespace model
{

class Fields
{
public:
private:
    std::string
};

class Config
{
    using FieldsStorage = std::vector<std::unique_ptr<Fields>>;
public:
    Config() = default;
    ~Config() = default;
private:
    FieldsStorage m_fields;
};

} // namespace model
