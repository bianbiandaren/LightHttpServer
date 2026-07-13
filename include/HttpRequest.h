#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>

class HttpRequest {
public:
    bool parse(const std::string& rawRequest);

    const std::string& method() const;
    const std::string& uri() const;
    const std::string& version() const;

private:
    std::string method_;
    std::string uri_;
    std::string version_;
};

#endif