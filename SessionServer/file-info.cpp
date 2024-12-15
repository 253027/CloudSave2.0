#include "file-info.h"

#include <unistd.h>

FileInfo::FileInfo(const std::string &name, const std::string &hash, uint32_t size)
    : _name(name), _fileHash(hash), _size(size)
{
    ;
}

FileInfo::~FileInfo()
{
    ;
}
