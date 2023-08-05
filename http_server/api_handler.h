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
    bool IsCreateDocsRequest(const StringRequest& request) const; ///< проверяет отправляет ли клиент сформированный json-шаблон для генерации документов
    StringResponse HandleApiRequest(const StringRequest& request);
    void HandleJsonRecieve(const StringRequest& request); ///< получает json-строку и формирует документы
    void StartScript(const std::string& savePath, const std::string& jsonConfig); ///< запускает скрипт
private:
    app::Application& m_app;
};

} // namespace http_handler
