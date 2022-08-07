#include "yahttp.hpp"

namespace YaHTTP {

  template class AsyncLoader<Request>;
  template class AsyncLoader<Response>;

  bool isspace(char c) {
    return std::isspace(c) != 0;
  }

  bool isspace(char c, const std::locale& loc) {
    return std::isspace(c, loc);
  }

  bool isxdigit(char c) {
    return std::isxdigit(c) != 0;
  }

  bool isxdigit(char c, const std::locale& loc) {
    return std::isxdigit(c, loc);
  }

  bool isdigit(char c) {
    return std::isdigit(c) != 0;
  }

  bool isdigit(char c, const std::locale& loc) {
    return std::isdigit(c, loc);
  }

  bool isalnum(char c) {
    return std::isalnum(c) != 0;
  }

  bool isalnum(char c, const std::locale& loc) {
    return std::isalnum(c, loc);
  }

  void HTTPBase::write(std::ostream& os) const {
    if (kind == YAHTTP_TYPE_REQUEST) {
      std::ostringstream getparmbuf;
      std::string getparms;
      // prepare URL
      for(strstr_map_t::const_iterator i = getvars.begin(); i != getvars.end(); i++) {
        getparmbuf << Utility::encodeURL(i->first, false) << "=" << Utility::encodeURL(i->second, false) << "&";
      }
      if (getparmbuf.str().length() > 0) {
        std::string buf = getparmbuf.str();
        getparms = "?" + std::string(buf.begin(), buf.end() - 1);
      }
      else
        getparms = "";
      os << method << " " << url.path << getparms << " HTTP/" << versionStr(this->version);
    } else if (kind == YAHTTP_TYPE_RESPONSE) {
      os << "HTTP/" << versionStr(this->version) << " " << status << " ";
      if (statusText.empty())
        os << Utility::status2text(status);
      else
        os << statusText;
    }
    os << "\r\n";

    bool cookieSent = false;
    bool sendChunked = false;

    if (this->version > 10) { // 1.1 or better
      if (headers.find("content-length") == headers.end() && !this->is_multipart) {
        // must use chunked on response
        sendChunked = (kind == YAHTTP_TYPE_RESPONSE);
        if ((headers.find("transfer-encoding") != headers.end() && headers.find("transfer-encoding")->second != "chunked")) {
          throw YaHTTP::Error("Transfer-encoding must be chunked, or Content-Length defined");
        }
        if ((headers.find("transfer-encoding") == headers.end() && kind == YAHTTP_TYPE_RESPONSE)) {
          sendChunked = true;
          os << "Transfer-Encoding: chunked\r\n";
        }
      } else {
	sendChunked = false;
      }
    }

    // write headers
    strstr_map_t::const_iterator iter = headers.begin();
    while(iter != headers.end()) {
      if (iter->first == "host" && (kind != YAHTTP_TYPE_REQUEST || version < 10)) { iter++; continue; }
      if (iter->first == "transfer-encoding" && sendChunked) { iter++; continue; }
      std::string header = Utility::camelizeHeader(iter->first);
      if (header == "Cookie" || header == "Set-Cookie") cookieSent = true;
      os << Utility::camelizeHeader(iter->first) << ": " << iter->second << "\r\n";
      iter++;
    }
    if (version > 9 && !cookieSent && jar.cookies.size() > 0) { // write cookies
     if (kind == YAHTTP_TYPE_REQUEST) {
        bool first = true;
        os << "Cookie: ";
        for(strcookie_map_t::const_iterator i = jar.cookies.begin(); i != jar.cookies.end(); i++) {
          if (first)
            first = false;
          else
            os << "; ";
          os << Utility::encodeURL(i->second.name) << "=" << Utility::encodeURL(i->second.value);
        }
     } else if (kind == YAHTTP_TYPE_RESPONSE) {
        for(strcookie_map_t::const_iterator i = jar.cookies.begin(); i != jar.cookies.end(); i++) {
          os << "Set-Cookie: ";
          os << i->second.str() << "\r\n";
        }
      }
    }
    os << "\r\n";
    this->renderer(this, os, sendChunked);
  };

  std::ostream& operator<<(std::ostream& os, const Response &resp) {
    resp.write(os);
    return os;
  };

  std::istream& operator>>(std::istream& is, Response &resp) {
    YaHTTP::AsyncResponseLoader arl;
    arl.initialize(&resp);
    while(is.good()) {
      char buf[1024];
      is.read(buf, 1024);
      if (is.gcount()>0) { // did we actually read anything
        is.clear();
        if (arl.feed(std::string(buf, is.gcount())) == true) break; // completed
      }
    }
    // throw unless ready
    if (arl.ready() == false)
      throw ParseError("Was not able to extract a valid Response from stream");
    arl.finalize();
    return is;
  };

  std::ostream& operator<<(std::ostream& os, const Request &req) {
    req.write(os);
    return os;
  };

  std::istream& operator>>(std::istream& is, Request &req) {
    YaHTTP::AsyncRequestLoader arl;
    arl.initialize(&req);
    while(is.good()) {
      char buf[1024];
      is.read(buf, 1024);
      if (is.gcount() > 0) { // did we actually read anything
        is.clear();
        if (arl.feed(std::string(buf, is.gcount())) == true) break; // completed
      }
    }
    if (arl.ready() == false)
      throw ParseError("Was not able to extract a valid Request from stream");
    arl.finalize();
    return is;
  };
};
