# CloudSave2.0

CloudSave2.0 是一个基于 [muduo](https://github.com/chenshuo/muduo) 网络库、[nlohmann/json](https://github.com/nlohmann/json) 以及 [picohttpparser](https://github.com/h2o/picohttpparser) 库构建的云存储系统。该项目仅供学习和交流使用，旨在展示如何构建高效、稳定的网络服务以及文件传输解决方案。

## 项目结构

- **FileServer**  
  该目录下包含了文件服务模块的源代码。FileServer 主要负责处理文件的上传、下载、存储及管理。为了应对大文件传输的需求，FileServer 实现了分块传输和断点续传(未完成)的功能：
  - **分块传输**：利用 HTTP 的 `Transfer-Encoding: chunked` 协议，服务器将文件分割为多个数据块逐步发送，降低内存压力和传输延迟。
  - **断点续传(未完成)**：通过支持 Range 请求，确保在网络中断或其他异常情况下，客户端可以从上次传输结束的地方继续下载文件。
  
- **src**  
  此目录包含了项目核心业务逻辑和通用组件的源代码。src 文件夹中实现了系统中各个模块之间的交互接口、数据处理逻辑以及工具函数，为 CloudSave2.0 提供了稳定、高效的基础支持。

## 主要功能

- **高效文件传输**  
  支持大文件的分块传输和断点续传，确保在网络不稳定时依然能够高效完成文件下载和上传。

- **模块化设计**  
  src 目录提供了通用的业务逻辑和工具库，使得各个模块之间的代码复用性和扩展性得到了保证。

## 技术栈

- **C++**：主要开发语言
- **muduo**：高性能网络库
- **nlohmann/json**：现代 C++ JSON 库
- **picohttpparser**：轻量级 HTTP/HTTPS报文解析库

## 构建和运行

请参考仓库中的 [makefile](./makefile) 以及其他相关文档，按照如下步骤构建和运行各个服务模块：
1. 克隆仓库到本地：  
   ```bash
   git clone https://github.com/253027/CloudSave2.0.git
   cd CloudSave2.0
2. 使用仓库中提供的`makefile`进行构建
   ```bash
   make
3. 在仓库中的`FileServer`文件夹内提供了`file-server.sql`脚本，用于创建数据库、表结构及初始数据。使用以下命令导入数据库脚本（请将your_username替换为你的MySQL用户名）
   ```bash
   mysql -u your_username -p < ./FileServer/file-server.sql
4. 修改数据库`database.json`配置文件
   ```
   {
      "ip": "localhost",
      "port": 3306,
      "username": "your_username",
      "password": "your_password",
      "databasename": "your_databases_name",
      "maxsize": 8,
      "minsize": 4,
      "timeout": 3,
      "idletimeout": 60
   }
   ```
5. 启动服务
   ```
   ./run.py
## TODO:
- **1.支持注册功能**
- **2.支持HTTPS**
- **3.断点续传**

