#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"
#include "yahttp/router.hpp"

using namespace boost;

class RouteTargetHandler {
public:
  static std::map<std::string, bool> routes;

  static void NonHandler(YaHTTP::Request *req, YaHTTP::Response *resp) { };

  static void GlobHandler(YaHTTP::Request *req, YaHTTP::Response *resp) {
   if (req->parameters["everything"] == "truly and really/everything/there/is.json")
     routes[req->routeName] = true;
  }

  static void Handler(YaHTTP::Request *req, YaHTTP::Response *resp) {
    routes[req->routeName] = true;
  }
  static void ObjectHandler(YaHTTP::Request *req, YaHTTP::Response *resp) {
    if (req->parameters["object"] == "1234" &&
        req->parameters["attribute"] == "name" &&
        req->parameters["format"] == "json") 
      routes[req->routeName] = true;
  }
} rth;

std::map<std::string, bool> RouteTargetHandler::routes;

struct RouterFixture {
  RouterFixture() { 
    BOOST_TEST_MESSAGE("Setup router");
    YaHTTP::Router::Get("/glob/<*everything>", rth.GlobHandler, "glob_get");
    YaHTTP::Router::Get("/test/<object>/<attribute>.<format>", rth.ObjectHandler, "object_attribute_format_get");
    YaHTTP::Router::Patch("/test/<object>/<attribute>.<format>", rth.Handler, "object_attribute_format_update");
    YaHTTP::Router::Get("/test", rth.Handler, "test_index");
    YaHTTP::Router::Get("/", rth.Handler, "root_path");

    // reset routes to false
    BOOST_FOREACH(YaHTTP::TRoute route, YaHTTP::Router::GetRoutes()) {
      rth.routes[std::get<3>(route)] = false;
    }

    // print routes
//    YaHTTP::Router::PrintRoutes(std::cout);

    RouteTargetHandler::routes.clear();
  };

  ~RouterFixture() {
    BOOST_TEST_MESSAGE("Clear router");
    YaHTTP::Router::Clear();
  }

};

BOOST_FIXTURE_TEST_SUITE( test_router, RouterFixture )

BOOST_AUTO_TEST_CASE( test_router_basic ) {
  // setup request
  YaHTTP::Request req;
  YaHTTP::Response resp;
  YaHTTP::THandlerFunction func = rth.NonHandler;
  req.setup("get", "http://test.org/");
  
  BOOST_CHECK(YaHTTP::Router::Route(&req, func));
  func(&req, &resp);

  // check if it was hit
  BOOST_CHECK(rth.routes["root_path"]);
};
 
BOOST_AUTO_TEST_CASE( test_router_object ) {
  // setup request
  YaHTTP::Request req;
  YaHTTP::Response resp;
  YaHTTP::THandlerFunction func = rth.NonHandler;
  req.setup("get", "http://test.org/test/1234/name.json");

  BOOST_CHECK(YaHTTP::Router::Route(&req, func));
  func(&req, &resp);

  // check if it was hit
  BOOST_CHECK(rth.routes["object_attribute_format_get"]);
};

BOOST_AUTO_TEST_CASE( test_router_glob ) {
  YaHTTP::Request req;
  YaHTTP::Response resp;
  YaHTTP::THandlerFunction func = rth.NonHandler;
  req.setup("get", "http://test.org/glob/truly and really/everything/there/is.json");

  BOOST_CHECK(YaHTTP::Router::Route(&req, func));
  func(&req, &resp);

  // check if it was hit
  BOOST_CHECK(rth.routes["glob_get"]);
};

BOOST_AUTO_TEST_CASE( test_router_urlFor ) {
  YaHTTP::strstr_map_t params;

  params["object"] = "1234";
  params["attribute"] = "name";
  params["format"] = "json";
  std::pair<std::string, std::string> route = YaHTTP::Router::URLFor("object_attribute_format_get", params);

  BOOST_CHECK_EQUAL(route.first, "GET");
  BOOST_CHECK_EQUAL(route.second, "/test/1234/name.json");

  params.clear();

  params["everything"] = "truly and really/everything/there/is.json";
  route = YaHTTP::Router::URLFor("glob_get", params);

  BOOST_CHECK_EQUAL(route.first, "GET");
  BOOST_CHECK_EQUAL(route.second, "/glob/truly%20and%20really/everything/there/is.json");
}

BOOST_AUTO_TEST_CASE( test_router_missing_route ) {
  YaHTTP::Request req;
  YaHTTP::Response resp;
  YaHTTP::THandlerFunction func = rth.NonHandler;
  req.setup("get", "http://test.org/missing/route");

  BOOST_CHECK(!YaHTTP::Router::Route(&req, func));
}

BOOST_AUTO_TEST_CASE( test_router_missing_urlFor ) {
  YaHTTP::strstr_map_t params;
  BOOST_CHECK_THROW((void)YaHTTP::Router::URLFor("object_attribute_format", params), YaHTTP::Error);
}

BOOST_AUTO_TEST_CASE( test_router_print_routes ) {
  std::ostringstream dest;
  YaHTTP::Router::PrintRoutes(dest);
  std::string tmp = dest.str();
  boost::erase_all(tmp, " ");

  BOOST_CHECK_EQUAL(tmp, "GET/glob/<*everything>glob_get\n\
GET/test/<object>/<attribute>.<format>object_attribute_format_get\n\
PATCH/test/<object>/<attribute>.<format>object_attribute_format_update\n\
GET/testtest_index\n\
GET/root_path\n\
");
};

}
