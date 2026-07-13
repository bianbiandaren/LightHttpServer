# LightHttpServer

一个使用 C++17 和 Linux Socket 实现的轻量级 HTTP 静态文件服务器。

## 当前功能

- 支持 HTTP GET 请求
- 支持静态 HTML、CSS、JavaScript、图片文件
- 支持 200、400、404、501 状态码
- 支持 MIME 类型识别
- 支持配置服务器端口和网站根目录
- 基础目录遍历防护

## 编译

```bash
cmake -S . -B build
cmake --build build