#include "HttpRequest.h"

#include <sstream>

bool HttpRequest::parse(const std::string& rawRequest) {
    method_.clear();
    uri_.clear();
    version_.clear();

    if (rawRequest.empty()) {
        return false;
    }

    std::istringstream rawStream(rawRequest);
    std::string requestLine;
    std::getline(rawStream, requestLine);

    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream requestStream(requestLine);

    if (!(requestStream >> method_ >> uri_ >> version_)) {
        method_.clear();
        uri_.clear();
        version_.clear();
        return false;
    }

    if (
        version_ != "HTTP/1.0" &&
        version_ != "HTTP/1.1"
    ) {
        method_.clear();
        uri_.clear();
        version_.clear();
        return false;
    }

    return !method_.empty() && !uri_.empty();
}

const std::string& HttpRequest::method() const {
    return method_;
}

const std::string& HttpRequest::uri() const {
    return uri_;
}

const std::string& HttpRequest::version() const {
    return version_;
}
