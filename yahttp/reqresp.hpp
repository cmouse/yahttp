#include <functional>

#include <fstream>
#include <cctype>

#ifndef WIN32
#include <cstdio>
#include <unistd.h>
#endif

#include <algorithm>

#ifndef YAHTTP_MAX_REQUEST_SIZE
#define YAHTTP_MAX_REQUEST_SIZE 2097152
#endif

#ifndef YAHTTP_MAX_RESPONSE_SIZE
#define YAHTTP_MAX_RESPONSE_SIZE 2097152
#endif

#define YAHTTP_TYPE_REQUEST 1
#define YAHTTP_TYPE_RESPONSE 2

namespace YaHTTP {
  typedef std::map<std::string,Cookie,ASCIICINullSafeComparator> strcookie_map_t; //<! String to Cookie map

  typedef enum {
    urlencoded,
    multipart
  } postformat_t; //<! Enumeration of possible post encodings, url encoding or multipart

  /*! Base class for request and response */
  class HTTPBase {
  public:
    /*! Default renderer for request/response, simply copies body to response */
    class SendBodyRender {
    public:
      SendBodyRender() {};

      size_t operator()(const HTTPBase *doc, std::ostream& os, bool chunked) const {
        if (chunked) {
          std::string::size_type i,cl;
          for(i=0;i<doc->body.length();i+=1024) {
            cl = std::min(static_cast<std::string::size_type>(1024), doc->body.length()-i); // for less than 1k blocks
            os << std::hex << cl << std::dec << "\r\n";
            os << doc->body.substr(i, cl) << "\r\n";
          }
          os << 0 << "\r\n\r\n"; // last chunk
        } else {
          os << doc->body;
        }
        return doc->body.length();
      }; //<! writes body to ostream and returns length 
    };
    /* Simple sendfile renderer which streams file to ostream */
    class SendFileRender {
    public:
      SendFileRender(const std::string& path_) {
        this->path = path_;
      };
  
      size_t operator()(const HTTPBase *doc __attribute__((unused)), std::ostream& os, bool chunked) const {
        char buf[4096];
        size_t n,k;
        std::ifstream ifs(path, std::ifstream::binary);
        n = 0;

        while(ifs.good()) {
          ifs.read(buf, sizeof buf);
          n += (k = ifs.gcount());
          if (k > 0) {
            if (chunked) os << std::hex << k << std::dec << "\r\n";
            os.write(buf, k);
            if (chunked) os << "\r\n"; 
          }
        }
        if (chunked) os << 0 << "\r\n\r\n";
        return n;
      }; //<! writes file to ostream and returns length

      std::string path; //<! File to send
    };

    HTTPBase() {
      HTTPBase::initialize();
    };

    virtual void initialize() {
      kind = 0;
      status = 0;
      renderer = SendBodyRender();
      max_request_size = YAHTTP_MAX_REQUEST_SIZE;
      max_response_size = YAHTTP_MAX_RESPONSE_SIZE;
      url = "";
      method = "";
      statusText = "";
      jar.clear();
      headers.clear();
      parameters.clear();
      getvars.clear();
      postvars.clear();
      body = "";
      routeName = "";
      version = 11; // default to version 1.1
      is_multipart = false;
    }
protected:
    HTTPBase(const HTTPBase& rhs) {
      this->url = rhs.url; this->kind = rhs.kind;
      this->status = rhs.status; this->statusText = rhs.statusText;
      this->method = rhs.method; this->headers = rhs.headers;
      this->jar = rhs.jar; this->postvars = rhs.postvars;
      this->parameters = rhs.parameters; this->getvars = rhs.getvars;
      this->body = rhs.body; this->max_request_size = rhs.max_request_size;
      this->max_response_size = rhs.max_response_size; this->version = rhs.version;
      this->renderer = rhs.renderer;
      this->is_multipart = rhs.is_multipart;
    };
    virtual HTTPBase& operator=(const HTTPBase& rhs) {
      this->url = rhs.url; this->kind = rhs.kind;
      this->status = rhs.status; this->statusText = rhs.statusText;
      this->method = rhs.method; this->headers = rhs.headers;
      this->jar = rhs.jar; this->postvars = rhs.postvars;
      this->parameters = rhs.parameters; this->getvars = rhs.getvars;
      this->body = rhs.body; this->max_request_size = rhs.max_request_size;
      this->max_response_size = rhs.max_response_size; this->version = rhs.version;
      this->renderer = rhs.renderer;
      this->is_multipart = rhs.is_multipart;
      return *this;
    };
public:
    URL url; //<! URL of this request/response
    int kind; //<! Type of object (1 = request, 2 = response)
    int status; //<! status code 
    int version; //<! http version 9 = 0.9, 10 = 1.0, 11 = 1.1
    std::string statusText; //<! textual representation of status code
    std::string method; //<! http verb
    strstr_map_t headers; //<! map of header(s)
    CookieJar jar; //<! cookies 
    strstr_map_t postvars; //<! map of POST variables (from POST body)
    strstr_map_t getvars; //<! map of GET variables (from URL)
// these two are for Router
    strstr_map_t parameters; //<! map of route parameters (only if you use YaHTTP::Router)
    std::string routeName; //<! name of the current route (only if you use YaHTTP::Router)

    std::string body; //<! the actual content

    ssize_t max_request_size; //<! maximum size of request
    ssize_t max_response_size;  //<! maximum size of response
    bool is_multipart; //<! if the request is multipart, prevents Content-Length header
    std::function<size_t(const HTTPBase*,std::ostream&,bool)> renderer; //<! rendering function
    void write(std::ostream& os) const; //<! writes request to the given output stream

    strstr_map_t& GET() { return getvars; }; //<! acccessor for getvars
    strstr_map_t& POST() { return postvars; }; //<! accessor for postvars
    strcookie_map_t& COOKIES() { return jar.cookies; }; //<! accessor for cookies

    std::string versionStr(int version_) const {
      switch(version_) {
      case  9: return "0.9";
      case 10: return "1.0";
      case 11: return "1.1";
      default: throw YaHTTP::Error("Unsupported version");
      }
    };

    std::string str() const {
       std::ostringstream oss;
       write(oss);
       return oss.str();
    }; //<! return string representation of this object
  };

  /*! Response class, represents a HTTP Response document */
  class Response: public HTTPBase { 
  public:
    Response() { Response::initialize(); };
    Response(const HTTPBase& rhs): HTTPBase(rhs) {
      this->kind = YAHTTP_TYPE_RESPONSE;
    };
    Response& operator=(const HTTPBase& rhs) override {
      HTTPBase::operator=(rhs);
      this->kind = YAHTTP_TYPE_RESPONSE;
      return *this;
    };
    void initialize() override {
      HTTPBase::initialize();
      this->kind = YAHTTP_TYPE_RESPONSE;
    }
    void initialize(const HTTPBase& rhs) {
      HTTPBase::initialize();
      this->kind = YAHTTP_TYPE_RESPONSE;
      // copy SOME attributes
      this->url = rhs.url;
      this->method = rhs.method;
      this->jar = rhs.jar;
      this->version = rhs.version;
    }
    friend std::ostream& operator<<(std::ostream& os, const Response &resp);
    friend std::istream& operator>>(std::istream& is, Response &resp);
  };

  /* Request class, represents a HTTP Request document */
  class Request: public HTTPBase {
  public:
    Request() { Request::initialize(); };
    Request(const HTTPBase& rhs): HTTPBase(rhs) {
      this->kind = YAHTTP_TYPE_REQUEST;
    };
    Request& operator=(const HTTPBase& rhs) override {
      HTTPBase::operator=(rhs);
      this->kind = YAHTTP_TYPE_REQUEST;
      return *this;
    };
    void initialize() override {
      HTTPBase::initialize();
      this->kind = YAHTTP_TYPE_REQUEST;
    }
    void initialize(const HTTPBase& rhs) {
      HTTPBase::initialize();
      this->kind = YAHTTP_TYPE_REQUEST;
      // copy SOME attributes
      this->url = rhs.url;
      this->method = rhs.method;
      this->jar = rhs.jar;
      this->version = rhs.version;
    }
    void setup(const std::string& method_, const std::string& url_) {
      this->url.parse(url_);
      this->headers["host"] = this->url.host.find(":") == std::string::npos ? this->url.host : "[" + this->url.host + "]";
      this->method = method_;
      std::transform(this->method.begin(), this->method.end(), this->method.begin(), ::toupper);
      this->headers["user-agent"] = "YaHTTP v1.0";
    }; //<! Set some initial things for a request

    void preparePost(postformat_t format = urlencoded) {
      std::ostringstream postbuf;
      if (format == urlencoded) {
        for(strstr_map_t::const_iterator i = POST().begin(); i != POST().end(); i++) {
          postbuf << Utility::encodeURL(i->first, false) << "=" << Utility::encodeURL(i->second, false) << "&";
        }
        // remove last bit
        if (postbuf.str().length()>0) 
          body = postbuf.str().substr(0, postbuf.str().length()-1);
        else
          body = "";
        headers["content-type"] = "application/x-www-form-urlencoded; charset=utf-8";
      } else if (format == multipart) {
        headers["content-type"] = "multipart/form-data; boundary=YaHTTP-12ca543";
        this->is_multipart = true;
        for(strstr_map_t::const_iterator i = POST().begin(); i != POST().end(); i++) {
          postbuf << "--YaHTTP-12ca543\r\nContent-Disposition: form-data; name=\"" << Utility::encodeURL(i->first, false) << "\"; charset=UTF-8\r\nContent-Length: " << i->second.size() << "\r\n\r\n"
            << Utility::encodeURL(i->second, false) << "\r\n";
        }
        postbuf << "--";
        body = postbuf.str();
      }

      postbuf.str("");
      postbuf << body.length();
      // set method and change headers
      method = "POST";
      if (!this->is_multipart)
        headers["content-length"] = postbuf.str();
    }; //<! convert all postvars into string and stuff it into body

    friend std::ostream& operator<<(std::ostream& os, const Request &resp);
    friend std::istream& operator>>(std::istream& is, Request &resp);
  };

  /*! Asynchronous HTTP document loader */
  template <class T>
  class AsyncLoader {
  public:
    T* target; //<! target to populate
    int state; //<! reader state
    size_t pos; //<! reader position
    
    std::string buffer; //<! read buffer 
    bool chunked; //<! whether we are parsing chunked data
    int chunk_size; //<! expected size of next chunk
    std::ostringstream bodybuf; //<! buffer for body
    size_t maxbody; //<! maximum size of body
    size_t minbody; //<! minimum size of body
    bool hasBody; //<! are we expecting body

    void keyValuePair(const std::string &keyvalue, std::string &key, std::string &value); //<! key value pair parser helper

    void initialize(T* target_) {
      chunked = false; chunk_size = 0;
      bodybuf.str(""); minbody = 0; maxbody = 0;
      pos = 0; state = 0; this->target = target_;
      hasBody = false;
      buffer = "";
      this->target->initialize();
    }; //<! Initialize the parser for target and clear state

    //<! Feed data to the parser
    bool feed(const std::string& somedata) {
      buffer.append(somedata);
      while(state < 2) {
        int cr=0;
        pos = buffer.find_first_of("\n");
        // need to find CRLF in buffer
        if (pos == std::string::npos) return false;
        if (pos>0 && buffer[pos-1]=='\r')
          cr=1;
        std::string line(buffer.begin(), buffer.begin()+pos-cr); // exclude CRLF
        buffer.erase(buffer.begin(), buffer.begin()+pos+1); // remove line from buffer including CRLF

        if (state == 0) { // startup line
          if (target->kind == YAHTTP_TYPE_REQUEST) {
            std::string ver;
            std::string tmpurl;
            std::istringstream iss(line);
            iss >> target->method >> tmpurl >> ver;
            if (ver.size() == 0)
              target->version = 9;
            else if (ver.find("HTTP/0.9") == 0)
              target->version = 9;
            else if (ver.find("HTTP/1.0") == 0)
              target->version = 10;
            else if (ver.find("HTTP/1.1") == 0)
              target->version = 11;
            else
              throw ParseError("HTTP version not supported");
            // uppercase the target method
            std::transform(target->method.begin(), target->method.end(), target->method.begin(), ::toupper);
            target->url.parse(tmpurl);
            target->getvars = Utility::parseUrlParameters(target->url.parameters);
            state = 1;
          } else if(target->kind == YAHTTP_TYPE_RESPONSE) {
            std::string ver;
            std::istringstream iss(line);
            std::string::size_type pos1;
            iss >> ver >> target->status;
            std::getline(iss, target->statusText);
            pos1=0;
            while(pos1 < target->statusText.size() && YaHTTP::isspace(target->statusText.at(pos1))) pos1++;
            target->statusText = target->statusText.substr(pos1);
            if ((pos1 = target->statusText.find("\r")) != std::string::npos) {
              target->statusText = target->statusText.substr(0, pos1-1);
            }
            if (ver.size() == 0) {
              target->version = 9;
            } else if (ver.find("HTTP/0.9") == 0)
              target->version = 9;
            else if (ver.find("HTTP/1.0") == 0)
              target->version = 10;
            else if (ver.find("HTTP/1.1") == 0)
              target->version = 11;
            else
              throw ParseError("HTTP version not supported");
            state = 1;
          }
        } else if (state == 1) {
          std::string key,value;
          size_t pos1;
          if (line.empty()) {
            chunked = (target->headers.find("transfer-encoding") != target->headers.end() && target->headers["transfer-encoding"] == "chunked");
            state = 2;
            break;
          }
          // split headers
          if ((pos1 = line.find(":")) == std::string::npos) {
            throw ParseError("Malformed header line");
          }
          key = line.substr(0, pos1);
          value = line.substr(pos1 + 1);
          for(std::string::iterator it=key.begin(); it != key.end(); it++)
            if (YaHTTP::isspace(*it))
              throw ParseError("Header key contains whitespace which is not allowed by RFC");

          Utility::trim(value);
          std::transform(key.begin(), key.end(), key.begin(), ::tolower);
          // is it already defined

          if (key == "set-cookie" && target->kind == YAHTTP_TYPE_RESPONSE) {
            target->jar.parseSetCookieHeader(value);
          } else if (key == "cookie" && target->kind == YAHTTP_TYPE_REQUEST) {
            target->jar.parseCookieHeader(value);
          } else {
            if (key == "host" && target->kind == YAHTTP_TYPE_REQUEST) {
              // maybe it contains port?
              if ((pos1 = value.find(":")) == std::string::npos) {
                target->url.host = value;
              } else {
                target->url.host = value.substr(0, pos1);
                target->url.port = ::atoi(value.substr(pos1).c_str());
              }
            }
            if (target->headers.find(key) != target->headers.end()) {
              target->headers[key] = target->headers[key] + ";" + value;
            } else {
              target->headers[key] = value;
            }
          }
        }
      }

      minbody = 0;
      // check for expected body size
      if (target->kind == YAHTTP_TYPE_REQUEST) maxbody = target->max_request_size;
      else if (target->kind == YAHTTP_TYPE_RESPONSE) maxbody = target->max_response_size;
      else maxbody = 0;

      if (!chunked) {
        if (target->headers.find("content-length") != target->headers.end()) {
          std::istringstream maxbodyS(target->headers["content-length"]);
          maxbodyS >> minbody;
          maxbody = minbody;
        }
        if (minbody < 1) return true; // guess there isn't anything left.
        if (target->kind == YAHTTP_TYPE_REQUEST && static_cast<ssize_t>(minbody) > target->max_request_size) throw ParseError("Max request body size exceeded");
        else if (target->kind == YAHTTP_TYPE_RESPONSE && static_cast<ssize_t>(minbody) > target->max_response_size) throw ParseError("Max response body size exceeded");
      }

      if (maxbody == 0) hasBody = false;
      else hasBody = true;

      if (buffer.size() == 0) return ready();

      while(buffer.size() > 0) {
        if (chunked) {
          if (chunk_size == 0) {
            char buf[100];
            // read chunk length
            if ((pos = buffer.find('\n')) == std::string::npos) return false;
            if (pos > 99)
              throw ParseError("Impossible chunk_size");
            buffer.copy(buf, pos);
            buf[pos]=0; // just in case...
            buffer.erase(buffer.begin(), buffer.begin()+pos+1); // remove line from buffer
            if (sscanf(buf, "%x", &chunk_size) != 1) {
              throw ParseError("Unable to parse chunk size");
            }
            if (chunk_size == 0) { state = 3; break; } // last chunk
          } else {
            int crlf=1;
            if (buffer.size() < static_cast<size_t>(chunk_size+1)) return false; // expect newline
            if (buffer.at(chunk_size) == '\r') {
              if (buffer.size() < static_cast<size_t>(chunk_size+2) || buffer.at(chunk_size+1) != '\n') return false; // expect newline after carriage return
              crlf=2;
            } else if (buffer.at(chunk_size) != '\n') return false;
            std::string tmp = buffer.substr(0, chunk_size);
            buffer.erase(buffer.begin(), buffer.begin()+chunk_size+crlf);
            bodybuf << tmp;
            chunk_size = 0;
            if (buffer.size() == 0) break; // just in case
          }
        } else {
          if (bodybuf.str().length() + buffer.length() > maxbody)
            bodybuf << buffer.substr(0, maxbody - bodybuf.str().length());
          else
            bodybuf << buffer;
          buffer = "";
        }
      }

      if (chunk_size!=0) return false; // need more data

      return ready();
    };

    bool ready() {
     return (chunked == true && state == 3) || // if it's chunked we get end of data indication
             (chunked == false && state > 1 &&  
               (!hasBody || 
                 (bodybuf.str().size() <= maxbody && 
                  bodybuf.str().size() >= minbody)
               )
             ); 
    }; //<! whether we have received enough data
    void finalize() {
      bodybuf.flush();
      if (ready()) {
        strstr_map_t::iterator cpos = target->headers.find("content-type");
        if (cpos != target->headers.end() && Utility::iequals(cpos->second, "application/x-www-form-urlencoded", 32)) {
          target->postvars = Utility::parseUrlParameters(bodybuf.str());
        }
        target->body = bodybuf.str();
      }
      bodybuf.str("");
      this->target = NULL;
    }; //<! finalize and release target
  };

  /*! Asynchronous HTTP response loader */
  class AsyncResponseLoader: public AsyncLoader<Response> {
  };

  /*! Asynchronous HTTP request loader */
  class AsyncRequestLoader: public AsyncLoader<Request> {
  };

};
