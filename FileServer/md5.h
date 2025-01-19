#ifndef __FILE_MD5_H__
#define __FILE_MD5_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>

class FileMD5
{
public:
    FileMD5();

    std::string compute(const std::string &filename);

    void update(unsigned char *input, unsigned int inputlen);

    std::string generate();

private:
    void init();

    void final(unsigned char digest[16]);

    void transform(unsigned char block[64]);

    void encode(unsigned char *output, unsigned int *input, unsigned int len);

    void decode(unsigned int *output, unsigned char *input, unsigned int len);

    struct MD5CTX
    {
        unsigned int count[2];
        unsigned int state[4];
        unsigned char buffer[64];
    };

    MD5CTX _context;
    const int _readSize = 1024;
    const int _md5Size = 16;
    const int _md5Len = _md5Size * 2 + 1;
};

#endif //__FILE_MD5_H__
