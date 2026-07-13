#include "HttpRequest.h"

#include <sstream>

bool HttpRequest::parse(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);

    if (!(requestStream >> method_ >> uri_ >> version_)) {
        return false;
    }

    if (
        version_ != "HTTP/1.0" &&
        version_ != "HTTP/1.1"
    ) {
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