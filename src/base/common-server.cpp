#include "common-server.h"

bool CommonServer::initial() { return true; }

void CommonServer::start() {}

void CommonServer::stop() {}

void CommonServer::onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c) {}
