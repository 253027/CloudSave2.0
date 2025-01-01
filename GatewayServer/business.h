#ifndef __BUSINESS_H__
#define __BUSINESS_H__

#include "../src/singleton.h"
#include "../src/http-packet-parser.h"

class Business : public Singleton<Business>
{
public:
    void login(const mg::HttpRequest &request);
};

#endif // __BUSINESS_H__