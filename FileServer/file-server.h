#ifndef __FILE_SERVER_H__
#define __FILE_SERVER_H__

#include "../src/singleton.h"
#include "../src/tcp-server.h"
#include "../src/http-packet-parser.h"
#include "../src/mysql-connection-pool.h"

class FileServer : public Singleton<FileServer>
{
public:
    FileServer();

    ~FileServer();

    bool initial();

    void start();

    void stop();

    void onMessage(const mg::TcpConnectionPointer &a, mg::Buffer *b, mg::TimeStamp c);

private: // 业务处后面可以单独抽出成一个类
    enum class FILESTATE : uint16_t
    {
        FILE_LOGIN = 0,
        FILE_WAIT_INFO,
        FILE_UPLOAD,
        FILE_COMPLETE
    };

    bool main(const mg::HttpRequest &request);

    bool uploadPage(const mg::HttpRequest &request);

    bool upload(const mg::HttpRequest &request);

    bool waitFileInfo(const mg::HttpRequest &request);

    bool fileInfo(const mg::HttpRequest &request);

    bool login(const mg::HttpRequest &request);

private: // 服务器底层接口定义处
    /**
     * @brief Restful API注册接口处
     */
    void regist();

    /**
     * @param 加载一些资源配置
     */
    void loadSource();

    std::string _indexContent;       // 网站首页内容
    std::string _uploadIndexContent; // 上传页面内容
    std::shared_ptr<mg::TcpServer> _server;
    std::shared_ptr<mg::EventLoop> _loop;
};

#endif // __FILE_SERVER_H__