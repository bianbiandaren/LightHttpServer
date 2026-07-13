# LightHttpServer

LightHttpServer 是一个 C++ 课程设计项目，使用 Linux Socket API 实现一个简单的静态 HTTP 服务器。项目主要用于练习网络编程、HTTP 请求解析、静态文件读取和基础 Git 协作。

## 当前功能

- 读取 `config/server.conf` 中的端口和网站根目录配置
- 支持阻塞式短连接访问
- 支持 HTTP GET 请求
- 支持访问 `www` 目录下的 HTML、CSS、JavaScript、图片等静态文件
- 访问 `/` 时返回 `www/index.html`
- 支持 200、400、403、404、501 等基本状态码
- 根据文件扩展名设置简单的 MIME 类型
- 对路径中的 `..` 做简单检查，避免访问网站目录外的文件
- 支持 `Ctrl+C` 停止服务器

## 项目目录结构

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

运行后会读取：

```text
config/server.conf
```

默认配置示例：

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
curl -i http://127.0.0.1:8080/not-found.html
curl -i http://127.0.0.1:8080/../config/server.conf
curl -i -X POST http://127.0.0.1:8080/
```

可以看到正常页面、404 页面、403 禁止访问和 501 不支持方法等结果。

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

- 只支持 GET 请求，不支持 POST
- 每次只处理一个客户端连接，没有线程池
- 没有实现 Keep-Alive，每个请求处理后都会关闭连接
- 没有实现 HTTPS
- 没有完整解析 HTTP Header
- 路径安全检查比较简单，只检查是否包含 `..`
- 适合作为课程设计基础版本，不适合直接用于真实生产环境
