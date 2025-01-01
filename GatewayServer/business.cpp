#include "business.h"
#include "../src/log.h"

void Business::login(const mg::HttpRequest &request)
{
    auto a = request.getConnection();
    mg::HttpResponse response;
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");
    response.setBody("<html>Hello World!</html>");
    mg::HttpPacketParser::get().send(a, response);
}