# LightHttpServer

LightHttpServer 是一个使用 **C++17、Linux Socket API、非阻塞 I/O 和 epoll** 实现的轻量级静态 HTTP 服务器。

本项目主要用于学习 Linux 网络编程、HTTP 协议、文件系统调用、I/O 多路复用、Reactor 事件模型、CMake 构建以及基础 Git 协作。

项目不依赖第三方网络库，服务器的 Socket 创建、HTTP 请求解析、静态文件读取和响应发送等功能均自行实现。

---

## 一、项目功能

目前已经实现以下功能：

- 从 `config/server.conf` 读取端口和网站根目录
- 使用 Linux Socket API 创建 TCP 服务器
- 使用单线程 epoll Reactor 处理多个客户端连接
- 监听 Socket 和客户端 Socket 均采用非阻塞模式
- 支持 HTTP/1.0 和 HTTP/1.1 请求行解析
- 支持 HTTP `GET` 请求
- 其他 HTTP 方法返回 `501 Not Implemented`
- 支持 HTML、CSS、JavaScript、JSON、文本和图片等静态资源
- 访问 `/` 时自动返回 `www/index.html`
- 根据文件扩展名设置对应的 MIME 类型
- 支持以下 HTTP 状态码：
  - `200 OK`
  - `400 Bad Request`
  - `403 Forbidden`
  - `404 Not Found`
  - `501 Not Implemented`
- 使用 `stat` 判断文件是否存在及文件类型
- 使用 `open/read/close` 读取静态文件
- 处理 TCP 分段读取和部分发送
- 限制 HTTP 请求头大小，防止请求缓冲区无限增长
- 拒绝包含 `..` 或反斜杠的危险路径
- 使用短连接，请求处理完成后主动关闭客户端连接
- 支持 `Ctrl+C` 停止服务器并清理相关资源

---

## 二、开发与运行环境

本项目主要在以下环境中开发和测试：

```text
操作系统：Windows 11
Linux 环境：WSL2 Ubuntu
编辑器：Visual Studio Code
编译器：g++
C++ 标准：C++17
构建工具：CMake
版本管理：Git + GitHub
```

项目使用了 Linux 独有的 API，例如：

```text
epoll_create1
epoll_ctl
epoll_wait
fcntl
socket
bind
listen
accept
```

因此，项目需要在 Linux、WSL 或其他兼容 Linux API 的环境中编译运行，不能直接使用普通 Windows 编译器进行编译。

### 推荐开发方式

```text
Windows
├── Visual Studio Code
├── 浏览器
└── WSL2 Ubuntu
    ├── g++
    ├── CMake
    ├── Git
    └── LightHttpServer
```

在 VS Code 左下角看到：

```text
WSL: Ubuntu
```

说明当前项目正在 WSL 环境中开发。

---

## 三、环境安装

在 Ubuntu 或 WSL 中执行：

```bash
sudo apt update
sudo apt install -y build-essential cmake git gdb curl netcat-openbsd
```

各工具的作用：

| 工具 | 作用 |
|---|---|
| `g++` | 编译 C++ 源代码 |
| `cmake` | 生成并管理项目构建文件 |
| `git` | 版本管理与多人协作 |
| `gdb` | 调试 C++ 程序 |
| `curl` | 测试 HTTP 请求和响应 |
| `nc` | 构造非法或不完整的 HTTP 请求 |

检查安装结果：

```bash
g++ --version
cmake --version
git --version
gdb --version
curl --version
nc -h
```

---

## 四、项目目录

```text
LightHttpServer/
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── config/
│   └── server.conf
│
├── include/
│   ├── Config.h
│   ├── HttpRequest.h
│   ├── HttpResponse.h
│   ├── MimeType.h
│   ├── Server.h
│   └── Utils.h
│
├── src/
│   ├── Config.cpp
│   ├── HttpRequest.cpp
│   ├── HttpResponse.cpp
│   ├── MimeType.cpp
│   ├── Server.cpp
│   ├── Utils.cpp
│   └── main.cpp
│
├── www/
│   ├── index.html
│   ├── 400.html
│   ├── 403.html
│   ├── 404.html
│   ├── 501.html
│   ├── css/
│   │   └── style.css
│   ├── js/
│   │   └── main.js
│   └── images/
│       └── ...
│
└── build/
    └── CMake 生成的编译文件
```

> `build/` 是自动生成的编译目录，不需要手动修改，也不应上传到 GitHub。

---

## 五、各模块作用

### `Config`

负责读取：

```text
config/server.conf
```

目前支持：

```text
PORT
ROOT_DIR
```

如果配置文件不存在或某项没有填写，程序会使用默认值。

---

### `HttpRequest`

负责解析 HTTP 请求起始行，例如：

```http
GET /index.html HTTP/1.1
```

解析结果包括：

```text
method  = GET
uri     = /index.html
version = HTTP/1.1
```

---

### `HttpResponse`

负责构造完整的 HTTP 响应，包括：

```text
状态行
Content-Type
Content-Length
Connection
空行
响应体
```

例如：

```http
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: 256
Connection: close

<!DOCTYPE html>
...
```

---

### `MimeType`

根据文件扩展名返回相应的 MIME 类型。

例如：

| 扩展名 | MIME 类型 |
|---|---|
| `.html` | `text/html; charset=utf-8` |
| `.css` | `text/css; charset=utf-8` |
| `.js` | `application/javascript; charset=utf-8` |
| `.json` | `application/json; charset=utf-8` |
| `.txt` | `text/plain; charset=utf-8` |
| `.png` | `image/png` |
| `.jpg`、`.jpeg` | `image/jpeg` |
| `.gif` | `image/gif` |
| `.svg` | `image/svg+xml` |
| `.ico` | `image/x-icon` |

未知扩展名返回：

```text
application/octet-stream
```

---

### `Utils`

提供项目中使用的辅助函数，例如：

- 清除字符串首尾空白
- 判断路径是否安全
- 读取文件
- 处理简单的路径检查

---

### `Server`

服务器的核心模块，负责：

- 创建监听 Socket
- 设置端口复用
- 设置非阻塞模式
- 绑定 IP 和端口
- 启动监听
- 创建 epoll 实例
- 注册、修改和删除 epoll 事件
- 接收客户端连接
- 读取 HTTP 请求
- 构造 HTTP 响应
- 发送静态文件
- 关闭并清理连接

---

### `main.cpp`

程序入口，主要负责：

1. 读取配置文件
2. 创建 `Server` 对象
3. 注册停止信号
4. 启动服务器

具体的网络和 HTTP 处理逻辑不会全部写在 `main.cpp` 中。

---

## 六、配置文件

配置文件位置：

```text
config/server.conf
```

示例：

```conf
# 服务器监听端口
PORT=8080

# 静态资源根目录
ROOT_DIR=www
```

### 修改端口

例如改为：

```conf
PORT=8081
ROOT_DIR=www
```

重新启动服务器后，访问：

```text
http://127.0.0.1:8081/
```

---

## 七、编译方法

进入项目根目录：

```bash
cd /mnt/d/Code/LightHttpServer
```

首次编译：

```bash
cmake -S . -B build
cmake --build build
```

正常情况下最后会显示：

```text
[100%] Built target LightHttpServer
```

如果希望彻底清除旧的构建缓存：

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

以后只修改 `.cpp` 或 `.h` 文件时，一般执行：

```bash
cmake --build build
```

即可重新编译。

---

## 八、运行方法

必须在项目根目录运行：

```bash
./build/LightHttpServer
```

正常输出类似：

```text
Port: 8080
Root directory: www
Server is listening on port 8080
```

程序启动后终端不会立即返回，这是正常现象，说明服务器正在等待客户端请求。

停止服务器：

```text
Ctrl+C
```

---

## 九、浏览器访问

服务器启动后，在 Windows 浏览器中打开：

```text
http://127.0.0.1:8080/
```

或：

```text
http://localhost:8080/
```

正常情况下会显示：

```text
www/index.html
```

### 测试静态资源

首页：

```text
http://127.0.0.1:8080/
```

CSS：

```text
http://127.0.0.1:8080/css/style.css
```

JavaScript：

```text
http://127.0.0.1:8080/js/main.js
```

图片：

```text
http://127.0.0.1:8080/images/图片文件名.png
```

不存在的文件：

```text
http://127.0.0.1:8080/not-found.html
```

应显示自定义 `404` 页面。

---

## 十、HTTP 状态码测试

测试前请保证服务器正在运行。

### 1. 测试 `200 OK`

```bash
curl -i http://127.0.0.1:8080/
```

预期：

```text
HTTP/1.1 200 OK
```

---

### 2. 测试 `404 Not Found`

```bash
curl -i http://127.0.0.1:8080/not-exist.html
```

预期：

```text
HTTP/1.1 404 Not Found
```

---

### 3. 测试 `403 Forbidden`

使用 `--path-as-is`，防止 curl 自动整理路径：

```bash
curl --path-as-is -i \
http://127.0.0.1:8080/../../etc/passwd
```

预期：

```text
HTTP/1.1 403 Forbidden
```

响应中不能出现 `/etc/passwd` 的实际内容。

---

### 4. 测试 `501 Not Implemented`

```bash
curl -i -X POST http://127.0.0.1:8080/
```

也可以测试：

```bash
curl -i -X PUT http://127.0.0.1:8080/
```

预期：

```text
HTTP/1.1 501 Not Implemented
```

---

### 5. 测试 `400 Bad Request`

浏览器通常不会发送格式错误的 HTTP 请求，因此使用 `nc` 手动构造：

```bash
printf "abc\r\n\r\n" | nc 127.0.0.1 8080
```

或者：

```bash
printf "GET\r\n\r\n" | nc 127.0.0.1 8080
```

预期：

```text
HTTP/1.1 400 Bad Request
```

---

## 十一、静态资源和 MIME 测试

项目当前只实现 `GET`，没有实现 `HEAD`，所以不要使用：

```bash
curl -I
```

因为 `curl -I` 发送的是 `HEAD` 请求，服务器会返回 `501`。

应使用普通 GET，并只显示响应头：

### HTML

```bash
curl -s -D - -o /dev/null \
http://127.0.0.1:8080/index.html
```

预期：

```text
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
```

### CSS

```bash
curl -s -D - -o /dev/null \
http://127.0.0.1:8080/css/style.css
```

预期：

```text
Content-Type: text/css; charset=utf-8
```

### JavaScript

```bash
curl -s -D - -o /dev/null \
http://127.0.0.1:8080/js/main.js
```

预期：

```text
Content-Type: application/javascript; charset=utf-8
```

### 图片

假设图片路径为：

```text
www/images/logo.png
```

执行：

```bash
curl -s -D - -o /dev/null \
http://127.0.0.1:8080/images/logo.png
```

预期：

```text
HTTP/1.1 200 OK
Content-Type: image/png
```

也可以下载图片进行检查：

```bash
curl http://127.0.0.1:8080/images/logo.png \
-o downloaded-logo.png
```

---

## 十二、epoll Reactor 工作流程

服务器启动后的基本流程：

```text
创建监听 Socket
        ↓
设置 SO_REUSEADDR
        ↓
设置监听 Socket 为非阻塞
        ↓
bind 绑定端口
        ↓
listen 开始监听
        ↓
创建 epoll 实例
        ↓
监听 Socket 注册到 epoll
        ↓
epoll_wait 等待事件
```

### 监听 Socket 可读

说明有新的客户端连接：

```text
accept 客户端
        ↓
设置客户端 Socket 为非阻塞
        ↓
加入 epoll
        ↓
创建客户端连接状态
```

### 客户端 Socket 可读

```text
循环 recv
        ↓
把数据追加到请求缓冲区
        ↓
检测是否出现 \r\n\r\n
        ↓
解析 HTTP 请求
        ↓
生成 HTTP 响应
        ↓
将事件修改为 EPOLLOUT
```

### 客户端 Socket 可写

```text
循环 send
        ↓
记录已经发送的字节数
        ↓
如果返回 EAGAIN，等待下一次可写事件
        ↓
响应发送完成
        ↓
从 epoll 删除连接
        ↓
close 客户端 Socket
```

本项目使用短连接模型，因此响应完成后主动关闭连接。

---

## 十三、Linux API 使用情况

项目使用了以下 Linux API。

### 网络相关

```text
socket
setsockopt
bind
listen
accept
recv
send
close
```

### epoll 相关

```text
epoll_create1
epoll_ctl
epoll_wait
```

### 非阻塞设置

```text
fcntl
F_GETFL
F_SETFL
O_NONBLOCK
```

### 文件读取

```text
stat
open
read
close
```

可以使用下面的命令检查代码中的相关实现：

```bash
grep -R \
"socket(\|bind(\|listen(\|accept(" \
-n src include
```

```bash
grep -R \
"epoll_create\|epoll_create1\|epoll_ctl\|epoll_wait" \
-n src include
```

```bash
grep -R \
"O_NONBLOCK\|F_GETFL\|F_SETFL" \
-n src include
```

```bash
grep -R \
"stat(\|open(\|read(\|close(" \
-n src include
```

---

## 十四、并发测试

### 100 个请求，20 个并发

```bash
seq 1 100 | xargs -P 20 -I {} \
curl -s -o /dev/null -w "%{http_code}\n" \
http://127.0.0.1:8080/ \
| sort | uniq -c
```

理想结果：

```text
100 200
```

---

### 1000 个请求，50 个并发

```bash
seq 1 1000 | xargs -P 50 -I {} \
curl -s -o /dev/null -w "%{http_code}\n" \
http://127.0.0.1:8080/ \
| sort | uniq -c
```

理想结果：

```text
1000 200
```

---

### ApacheBench 测试（可选）

安装：

```bash
sudo apt install -y apache2-utils
```

执行：

```bash
ab -n 1000 -c 50 http://127.0.0.1:8080/
```

重点查看：

```text
Complete requests
Failed requests
Requests per second
Time per request
```

理想情况：

```text
Complete requests: 1000
Failed requests: 0
```

---

## 十五、慢客户端测试

该测试用于验证某个客户端没有发送完整请求时，不会阻塞其他客户端。

终端一执行：

```bash
nc 127.0.0.1 8080
```

只输入：

```http
GET / HTTP/1.1
```

暂时不要补充空行，也不要关闭连接。

然后在另一个终端执行：

```bash
curl -i http://127.0.0.1:8080/
```

如果第二个请求仍能立即返回：

```text
HTTP/1.1 200 OK
```

说明服务器能够同时管理多个客户端，单个慢连接不会阻塞整个事件循环。

---

## 十六、Ctrl+C 退出处理

在 Linux 中，按下：

```text
Ctrl+C
```

会向服务器发送：

```text
SIGINT
```

项目对该信号进行了处理，使服务器能够按照正常流程退出。

退出时主要完成：

- 停止 epoll 事件循环
- 关闭所有客户端 Socket
- 关闭监听 Socket
- 关闭 epoll 文件描述符
- 清理客户端连接状态

这种方式相比直接强制结束程序更加规范，可以减少资源未释放和端口暂时不可用等问题。

---

## 十七、Git 协作流程

### 首次获取项目

```bash
git clone git@github.com:bianbiandaren/LightHttpServer.git
cd LightHttpServer
```

### 每次开发前

```bash
git pull
```

### 修改并测试完成后

```bash
git status
git add .
git commit -m "说明本次完成的功能"
git push
```

多人协作时应尽量避免同时修改同一段代码。如果出现冲突，需要手动选择正确内容，并删除：

```text
<<<<<<<
=======
>>>>>>>
```

等冲突标记。

---

## 十八、修改文件后的操作

### 只修改网页资源

例如修改：

```text
www/index.html
www/css/style.css
www/js/main.js
www/images/*
```

不需要重新编译 C++，保存文件后刷新浏览器即可。

建议使用：

```text
Ctrl+F5
```

强制刷新，避免浏览器缓存旧文件。

### 修改 C++ 代码

例如修改：

```text
src/*.cpp
include/*.h
CMakeLists.txt
```

需要重新编译：

```bash
cmake --build build
```

然后停止旧服务器：

```text
Ctrl+C
```

再重新启动：

```bash
./build/LightHttpServer
```

---

## 十九、当前限制

目前项目仍存在以下限制：

- 只支持 `GET` 请求
- 不支持 `HEAD`
- 不支持 POST 请求体
- 没有完整解析所有 HTTP Header
- 不支持 HTTP Keep-Alive
- 每个请求完成后都会关闭连接
- 不支持 Chunked Transfer Encoding
- 没有线程池
- 没有文件缓存
- 没有使用 `sendfile` 或其他零拷贝技术
- 不支持 HTTPS
- 不支持 CGI、WebSocket 和动态网页
- URL 编码处理能力有限
- 路径安全检查主要满足课程设计要求
- 适合作为学习和课程设计项目，不适合直接部署到真实生产环境

---

## 二十、项目特点

### 可用性

项目能够通过浏览器真正访问网页，并加载 CSS、JavaScript 和图片等静态资源，不只是一个简单的 Socket 示例。

### 易用性

服务器可以通过 CMake 编译，通过配置文件修改端口和网站目录，使用方式较为简单。

### 效率

使用 epoll 和非阻塞 Socket，一个线程可以同时管理多个客户端连接，避免为每个客户端创建线程。

### 可维护性

项目按照配置、请求、响应、MIME、工具和服务器等模块进行划分，修改某个功能时不需要重写整个项目。

### 学习价值

项目不依赖现成 HTTP 框架，能够较完整地展示：

```text
TCP 连接
HTTP 请求解析
HTTP 响应构造
静态文件读取
epoll 事件循环
非阻塞 I/O
多人 Git 协作
```

等内容。