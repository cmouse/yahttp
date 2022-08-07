#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE unit

#include <clocale>
#include <boost/test/unit_test.hpp>

struct MyConfig {
  MyConfig() {
    std::setlocale(LC_ALL, "C");
  }
};

BOOST_GLOBAL_FIXTURE( MyConfig );
