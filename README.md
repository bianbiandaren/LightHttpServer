# LightHttpServer

LightHttpServer 是一个 C++ 课程设计项目，使用 Linux Socket API 和 epoll 实现一个简单的静态 HTTP 服务器。项目主要用于练习网络编程、HTTP 请求解析、静态文件读取、epoll 事件循环和基础 Git 协作。

## 当前功能

- 读取 `config/server.conf` 中的端口和网站根目录配置
- 使用单线程 epoll Reactor 处理连接
- 监听 socket 和客户端 socket 使用非阻塞模式
- 支持 HTTP GET 请求
- 其他 HTTP 方法返回 `501 Not Implemented`
- 支持访问 `www` 目录下的 HTML、CSS、JavaScript、图片等静态文件
- 访问 `/` 时返回 `www/index.html`
- 支持 `200`、`400`、`403`、`404`、`501` 等基本状态码
- 根据文件扩展名设置简单的 MIME 类型
- 使用 `stat` 检查文件，使用 `open/read/close` 读取文件
- 对包含 `..` 或反斜杠的路径返回 `403 Forbidden`
- 支持 `Ctrl+C` 停止服务器并清理资源

## 项目目录

```text
LightHttpServer/
├── CMakeLists.txt
├── README.md
├── config/
│   └── server.conf
├── include/
│   ├── Config.h
│   ├── HttpRequest.h
│   ├── HttpResponse.h
│   ├── MimeType.h
│   ├── Server.h
│   └── Utils.h
├── src/
│   ├── Config.cpp
│   ├── HttpRequest.cpp
│   ├── HttpResponse.cpp
│   ├── MimeType.cpp
│   ├── Server.cpp
│   ├── Utils.cpp
│   └── main.cpp
└── www/
    ├── index.html
    ├── 403.html
    ├── 404.html
    ├── css/
    │   └── style.css
    └── js/
        └── main.js
```

## 编译方法

```bash
cd /mnt/d/Code/LightHttpServer
cmake -S . -B build
cmake --build build
```

## 运行方法

```bash
./build/LightHttpServer
```

运行时会读取：

```text
config/server.conf
```

配置示例：

```text
PORT=8080
ROOT_DIR=www
```

## 浏览器访问方法

服务器启动后，在浏览器中打开：

```text
http://127.0.0.1:8080/
```

如果页面显示“服务器运行成功”，说明首页可以正常访问。点击页面上的按钮，如果出现提示文字，说明 CSS 和 JavaScript 文件也能正常加载。

## curl 测试方法

```bash
curl -i http://127.0.0.1:8080/
curl -i http://127.0.0.1:8080/index.html
curl -i http://127.0.0.1:8080/css/style.css
curl -i http://127.0.0.1:8080/js/main.js
curl -i http://127.0.0.1:8080/not-found.html
curl -i http://127.0.0.1:8080/../config/server.conf
curl -i -X POST http://127.0.0.1:8080/
```

可以看到正常页面、静态资源、404 页面、403 禁止访问和 501 不支持方法等结果。

## epoll Reactor 简单流程

服务器启动后先创建监听 socket，设置端口复用和非阻塞模式，然后创建 epoll 实例，把监听 socket 注册到 epoll 中。

主循环中通过 `epoll_wait` 等待事件：

- 监听 socket 可读：循环 `accept` 新客户端，直到遇到 `EAGAIN` 或 `EWOULDBLOCK`
- 客户端可读：循环 `recv`，把数据追加到请求缓冲区
- 请求头出现空行：说明请求头读取完成，解析请求并生成响应
- 客户端可写：循环 `send`，处理部分发送
- 响应发送完成：从 epoll 删除客户端，关闭连接
- 出现错误或关闭事件：清理客户端连接

本项目使用短连接模型，所以每个响应发送完成后都会主动关闭客户端 socket。

## 两人协作时的简单 Git 流程

两个人合作时，可以先约定好每次只改自己负责的文件，减少冲突。

```bash
git pull
git status
git add 修改过的文件
git commit -m "说明这次修改内容"
git push
```

如果出现冲突，先打开冲突文件，删除 Git 自动插入的冲突标记，确认保留正确代码后再提交。

## 当前限制

- 只支持 GET 请求，不支持 POST 请求体
- 没有实现线程池
- 没有实现 Keep-Alive，每个请求处理后都会关闭连接
- 没有实现 HTTPS
- 没有完整解析 HTTP Header
- 没有实现缓存系统
- 路径安全检查比较基础
- 适合作为课程设计版本，不适合直接用于真实生产环境
