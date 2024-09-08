#ifndef __MG_CONNECTION_H__
#define __MG_CONNECTION_H__

#include <assert.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>

#include "mg_socket.h"

namespace mg
{
    class Connection : noncopyable
    {
    public:
        explicit Connection(const std::string &ip, int port, int domain, int type);

        ~Connection();

    private:
        Socket socket;
    };
};

#endif //__MG_CONNECTION_H__