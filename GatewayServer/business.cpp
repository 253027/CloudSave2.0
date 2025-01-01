#include "business.h"
#include "../src/log.h"
#include "../src/json-extract.h"

using json = nlohmann::json;

bool Business::main(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    mg::HttpResponse response;
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");
    response.setBody("<html>Hello World!</html>");
    mg::HttpPacketParser::get().send(a, response);
    return true;
}

bool Business::login(const mg::HttpRequest &request)
{
    auto con = request.getConnection();
    if (!con)
        return false;

    json js;
    mg::HttpResponse response;
    if (!this->parse(js, request.body()))
        return false;

    std::string name, password;
    if (!mg::JsonExtract::extract(js, "name", name, mg::JsonExtract::STRING))
        return false;

    return true;
}

bool Business::parse(nlohmann::json &js, const std::string &data)
{
    try
    {
        js = json::parse(data);
    }
    catch (const json::parse_error &e)
    {
        return false;
    }
    return true;
}