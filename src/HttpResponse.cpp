#include "HttpResponse.h"

#include <sstream>

void HttpResponse::setStatus(int statusCode) {
    statusCode_ = statusCode;
}

void HttpResponse::setContentType(
    const std::string& contentType
) {
    contentType_ = contentType;
}

void HttpResponse::setBody(const std::string& body) {
    body_ = body;
}

std::string HttpResponse::build() const {
    std::ostringstream response;

    response
        << "HTTP/1.1 "
        << statusCode_
        << ' '
        << statusText(statusCode_)
        << "\r\n";

    response
        << "Content-Type: "
        << contentType_
        << "\r\n";

    response
        << "Content-Length: "
        << body_.size()
        << "\r\n";

    response << "Connection: close\r\n";
    response << "\r\n";
    response << body_;

    return response.str();
}

std::string HttpResponse::statusText(int statusCode) {
    switch (statusCode) {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 501:
            return "Not Implemented";
        default:
            return "Unknown";
    }
}
