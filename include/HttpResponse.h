#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>

class HttpResponse {
public:
    void setStatus(int statusCode);
    void setContentType(const std::string& contentType);
    void setBody(const std::string& body);

    std::string build() const;

private:
    static std::string statusText(int statusCode);

    int statusCode_ = 200;
    std::string contentType_ = "text/plain; charset=utf-8";
    std::string body_;
};

#endif