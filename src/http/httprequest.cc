#include "httprequest.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const {
    if (header_.find("Connection") != header_.end()) {
        return header_.at("Connection") == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::parse(Buffer &buff) {
    const char CRLF[] = "\r\n";
    if (buff.ReadableBytes() <= 0) {
        return false;
    }
    while (buff.ReadableBytes() > 0 && state_ != FINISH) {
        const char *line_end = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF+2);
        std::string line(buff.Peek(), line_end);
        switch (state_) {
            case REQUEST_LINE:
                if (!ParseRequestLine_(line)) {
                    return false;
                }
                ParsePath_();
                break;
            case HEADERS:
                ParseHeader_(line);
                if (buff.ReadableBytes() <= 2) {
                    state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        if (line_end == buff.BeginWrite()) {
            break;
        }
        buff.RetrieveUntil(line_end+2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

std::string HttpRequest::path() const {
    return path_;
}

std::string& HttpRequest::path() {
    return path_;
}

std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if (post_.find(key) != post_.end()) {
        return post_.at(key);
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if (post_.find(key) != post_.end()) {
        return post_.at(key);
    }
    return "";   
}

bool HttpRequest::ParseRequestLine_(const std::string& line) {
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch submatch;
    if (std::regex_match(line, submatch, pattern)) {
        method_ = submatch[1];
        path_ = submatch[2];
        version_ = submatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader_(const std::string& line) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch submatch;
    if (std::regex_match(line, submatch, pattern)) {
        header_[submatch[1]] = submatch[2];
    } else {
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const std::string& line) {
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::ParsePath_() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto &item: DEFAULT_HTML) {
            if (item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::ParsePost_() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_();
        if (DEFAULT_HTML_TAG.find(path_) != DEFAULT_HTML_TAG.end()) {
            int tag = DEFAULT_HTML_TAG.at(path_);
            LOG_DEBUG("Tag;%d", tag);
            bool is_login = tag == 1;
            if (UserVerify(post_["username"], post_["password"], is_login)) {
                path_ = "/welcome.html";
            } else {
                path_ = "/error.html";
            }
        }
    }

}

void HttpRequest::ParseFromUrlencoded_() {
    if (body_.size() == 0) {
        return;
    }
    std::string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for (; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;            
            case '+':
                body_[i] = ' ';
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;            
        }
    }
}

int HttpRequest::ConverHex(char ch) {
    if (ch >= 'A' && ch <= 'F') {
        return ch-'A'+10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch-'a'+10;
    }
    return ch;
}