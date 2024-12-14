#include "file-info.h"

#include <unistd.h>

FileInfo::FileInfo()
{
    this->_fd = ::open("/dev/null", O_RDWR | O_CLOEXEC);
    

}

FileInfo::~FileInfo()
{
    TEMP_FAILURE_RETRY(::close(this->_fd));
}

