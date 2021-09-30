EXTRA_PROGRAMS=test
EXTRA_DIST=request-get-cookies-ok.txt request-get-incomplete.txt request-get-ok.txt \
           request-issue-7.txt request-post-ok.txt response-binary.txt response-chunked-body.txt \
           response-chunked-headers.txt response-google-200.txt response-incomplete.txt request-chunked.txt

test_CFLAGS=$(RELRO_CFLASG) $(PIE_CFLAGS) -I$(top_srcdir) $(BOOST_CPPFLAGS)
test_CXXFLAGS=$(RELRO_CFLASG) $(PIE_CFLAGS) -I$(top_srcdir) $(BOOST_CPPFLAGS)
test_LDADD=$(RELRO_LDFLAGS) $(PIE_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) ../yahttp/libyahttp.la
test_SOURCES=md5.h md5.c test-main.cpp test-md5.cpp test-utility.cpp test-url.cpp test-cookie.cpp test-request.cpp test-response.cpp test-router.cpp

TESTS=test
TESTS_ENVIRONMENT = env BOOST_TEST_LOG_LEVEL=all
