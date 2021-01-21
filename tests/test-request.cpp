#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_request)

BOOST_AUTO_TEST_CASE(test_request_parse_get)
{
std::ifstream ifs("request-get-ok.txt");
YaHTTP::Request req;
ifs >> req;

BOOST_CHECK_EQUAL(req.method, "GET");
BOOST_CHECK_EQUAL(req.version, 11);
BOOST_CHECK_EQUAL(req.url.path, "/");
BOOST_CHECK_EQUAL(req.url.host, "test.org");
BOOST_CHECK_EQUAL(req.headers["accept-encoding"], "gzip, deflate");
}

BOOST_AUTO_TEST_CASE(test_request_parse_get_arl)
{
std::ifstream ifs("request-get-ok.txt");
YaHTTP::Request req;
YaHTTP::AsyncRequestLoader arl;
arl.initialize(&req);

while(ifs.good()) {
char buf[1024];
ifs.read(buf, 1024);
if (ifs.gcount()) { // did we actually read anything
ifs.clear();
if (arl.feed(std::string(buf, ifs.gcount())) == true) break; // completed
}
}
BOOST_CHECK(arl.ready());

arl.finalize();

BOOST_CHECK_EQUAL(req.method, "GET");
BOOST_CHECK_EQUAL(req.url.path, "/");
BOOST_CHECK_EQUAL(req.url.host, "test.org");
}

BOOST_AUTO_TEST_CASE(test_request_parse_incomplete_fail) {
std::ifstream ifs("request-get-incomplete.txt");
YaHTTP::Request req;
BOOST_CHECK_THROW(ifs >> req , YaHTTP::ParseError);
}

BOOST_AUTO_TEST_CASE(test_request_parse_post)
{
std::ifstream ifs("request-post-ok.txt");
YaHTTP::Request req;
ifs >> req;

BOOST_CHECK_EQUAL(req.method, "POST");
BOOST_CHECK_EQUAL(req.url.path, "/test");
BOOST_CHECK_EQUAL(req.url.host, "test.org");

BOOST_CHECK_EQUAL(req.POST()["Hi"], "Moi");
BOOST_CHECK_EQUAL(req.POST()["M"], "Bää");
BOOST_CHECK_EQUAL(req.POST()["Bai"], "Kai");
BOOST_CHECK_EQUAL(req.POST()["Li"], "Ann");
}

BOOST_AUTO_TEST_CASE(test_request_parse_cookies) 
{
std::ifstream ifs("request-get-cookies-ok.txt");
YaHTTP::Request req;
ifs >> req;

BOOST_CHECK_EQUAL(req.COOKIES()["type"].value, "data");
BOOST_CHECK_EQUAL(req.COOKIES()["more"].value, "values");
BOOST_CHECK_EQUAL(req.COOKIES()["just"].value, "like this");
BOOST_CHECK_EQUAL(req.COOKIES()["even"].value, "more");
BOOST_CHECK_EQUAL(req.COOKIES()["cookies"].value, "kääkkä");
}

BOOST_AUTO_TEST_CASE(test_request_build_post) 
{
YaHTTP::Request req;
req.setup("POST", "http://example.org/test");
req.POST()["one"] = "w";
req.preparePost();

BOOST_CHECK_EQUAL(req.str(), 
"POST /test HTTP/1.1\r\n\
Content-Length: 5\r\n\
Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n\
Host: example.org\r\n\
User-Agent: YaHTTP v1.0\r\n\
\r\n\
one=w"
);
}

BOOST_AUTO_TEST_CASE(test_request_issue_7)
{
YaHTTP::Request req;
std::ifstream ifs("request-issue-7.txt");

BOOST_CHECK_THROW(ifs>>req, YaHTTP::ParseError);
}

BOOST_AUTO_TEST_CASE(test_request_chunked)
{
YaHTTP::Request req;
std::ifstream ifs("request-chunked.txt");
ifs>>req;
BOOST_CHECK_EQUAL(req.body, "{\"login\":\"cmouse\",\"pwhash\":\"1234\",\"remote\":\"127.0.0.1\"}");
}

BOOST_AUTO_TEST_CASE(test_request_version)
{
YaHTTP::Request req;
req.setup("GET", "http://example.org");
req.version = 9;

BOOST_CHECK_EQUAL(req.str(),
"GET / HTTP/0.9\r\n\
User-Agent: YaHTTP v1.0\r\n\r\n");

req.version = 10;
BOOST_CHECK_EQUAL(req.str(), 
"GET / HTTP/1.0\r\n\
Host: example.org\r\n\
User-Agent: YaHTTP v1.0\r\n\r\n");

req.version = 11;
BOOST_CHECK_EQUAL(req.str(),
"GET / HTTP/1.1\r\n\
Host: example.org\r\n\
User-Agent: YaHTTP v1.0\r\n\r\n");

req.version = 1;

BOOST_CHECK_THROW(req.str(), YaHTTP::Error);
}

BOOST_AUTO_TEST_CASE(test_post_empty)
{
YaHTTP::Request req;
req.version = 11;
req.setup("POST", "http://example.org/test");
req.preparePost();

BOOST_CHECK_EQUAL(req.str(),
"POST /test HTTP/1.1\r\n\
Content-Length: 0\r\n\
Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n\
Host: example.org\r\n\
User-Agent: YaHTTP v1.0\r\n\r\n");
}

BOOST_AUTO_TEST_CASE(test_post_multipart)
{
YaHTTP::Request req;
req.version = 11;
req.setup("POST", "http://example.org/test");
req.POST()["hello"] = "world";
req.POST()["tree"] = "apple";
req.preparePost(YaHTTP::multipart);

BOOST_CHECK_EQUAL(req.str(),
"POST /test HTTP/1.1\r\n\
Content-Type: multipart/form-data; boundary=YaHTTP-12ca543\r\n\
Host: example.org\r\n\
User-Agent: YaHTTP v1.0\r\n\
\r\n\
--YaHTTP-12ca543\r\n\
Content-Disposition: form-data; name=\"hello\"; charset=UTF-8\r\n\
Content-Length: 5\r\n\
\r\n\
world\r\n\
--YaHTTP-12ca543\r\n\
Content-Disposition: form-data; name=\"tree\"; charset=UTF-8\r\n\
Content-Length: 5\r\n\
\r\n\
apple\r\n\
--");
}

BOOST_AUTO_TEST_CASE(test_get_vars)
{
YaHTTP::Request req;
req.version = 11;
req.setup("GET", "http://example.org/test");
req.GET()["hello"] = "world";
req.GET()["tree"] = "apple";
BOOST_CHECK_EQUAL(req.str(),
"GET /test?hello=world&tree=apple HTTP/1.1\r\n\
Host: example.org\r\n\
User-Agent: YaHTTP v1.0\r\n\r\n");
}

}
BOOST_AUTO_TEST_CASE(test_get_host_ipv6_literal_with_port) 
{
YaHTTP::Request req;
req.version = 11;
req.setup("GET", "http://[2001:db8::1]:5555/test");
req.GET()["hello"] = "world";
req.GET()["tree"] = "apple";
BOOST_CHECK_EQUAL(req.str(),
"GET /test?hello=world&tree=apple HTTP/1.1\r\n\
Host: [2001:db8::1]\r\n\
User-Agent: YaHTTP v1.0\r\n\r\n");
}
