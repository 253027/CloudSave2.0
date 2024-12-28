#include "file-server.h"

FileServer::FileServer()
{
    ;
}

FileServer::~FileServer()
{
    ;
}

void FileServer::initial()
{
    _loop.reset(new mg::EventLoop("FileServer"));
}

void FileServer::start()
{
    ;
}

void FileServer::stop()
{
    ;
}

void FileServer::onMessage()
{
    ;
}
