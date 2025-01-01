#include "business.h"
#include "session-server-client.h"
#include "../src/tcp-connection.h"
#include "../src/log.h"
#include "../src/json-extract.h"
#include "../protocal/protocal-session.h"

#include <fstream>
using json = nlohmann::json;

bool Business::main(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    mg::HttpResponse response;
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");

    std::ifstream file("./source/index.html");
    if (file.is_open())
    {
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        response.setBody(data);
    }
    else
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
    if (!this->parse(js, request.body()))
        return false;

    js["connection-name"] = con->name();
    SessionClient::get().sendToServer(Protocal::SessionCommand(Protocal::SessionType::LOGIN).serialize(js.dump()));
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