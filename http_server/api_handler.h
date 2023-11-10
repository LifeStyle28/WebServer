#pragma once

#include "application.h"
#include "http_types.h"

namespace http_handler
{

class ResponseBuilder;

class ApiHandler
{
public:
    explicit ApiHandler(app::Application& app);

    bool IsApiRequest(const StringRequest& request) const;
    bool IsOptionsRequest(const StringRequest& request) const;
    StringResponse HandleApiRequest(const StringRequest& request);
private:
    app::Application& m_app;
};

} // namespace http_handler
