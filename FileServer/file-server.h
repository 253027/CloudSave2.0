#ifndef __FILE_SERVER_H__
#define __FILE_SERVER_H__

#include "../src/singleton.h"
#include "../src/tcp-server.h"
#include "../src/json.hpp"
#include "../src/http-packet-parser.h"
#include "../src/mysql-connection-pool.h"

class FileInfo;
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
        UNVERIFY,
        VERIFY
    };

    bool main(const mg::HttpRequest &request);

    bool uploadPage(const mg::HttpRequest &request);

    bool upload(const mg::HttpRequest &request);

    bool waitFileInfo(const mg::HttpRequest &request);

    bool fileInfo(const mg::HttpRequest &request);

    bool login(const mg::HttpRequest &request);

    void judgeFileMD5(std::shared_ptr<FileInfo> &file, mg::TcpConnectionPointer &connection, bool needCalc = false);

    void updateDataBase(std::shared_ptr<FileInfo> &file, const std::string &name);

    bool download(const mg::HttpRequest &request);

    void streamSend(int index, const mg::TcpConnectionPointer &connection, const std::string &filename);

private: // 服务器底层接口定义处
    /**
     * @brief Restful API注册接口处
     */
    void regist();

    /**
     * @param 加载一些资源配置
     */
    void loadSource();

    /**
     * @brief 连接断开时执行资源释放操作
     */
    void onConnectionStateChanged(const mg::TcpConnectionPointer &connection);

    std::string _indexContent;                          // 网站首页内容
    std::string _uploadIndexContent;                    // 上传页面内容
    nlohmann::json _config;                             // 配置文件
    std::shared_ptr<mg::EventLoopThreadPool> _calcPool; // 文件校验线程池
    std::shared_ptr<mg::TcpServer> _server;
    std::shared_ptr<mg::EventLoop> _loop;
};

#endif // __FILE_SERVER_H__