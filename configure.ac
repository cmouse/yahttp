AC_PREREQ([2.61])

AC_INIT([yahttp], [git])

AC_CONFIG_SRCDIR([yahttp/reqresp.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([yahttp/yahttp-config.h])

AM_INIT_AUTOMAKE([foreign tar-ustar -Wno-portability subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([], [optional])
AM_PROG_CC_C_O
AC_PROG_MAKE_SET
LT_INIT
AC_CC_PIE
AC_LD_RELRO
AX_CODE_COVERAGE

AC_CHECK_FUNCS([localtime_r])

AC_CHECK_MEMBER(struct tm.tm_gmtoff,
  [AC_DEFINE(HAVE_TM_GMTOFF, 1,
     [Define if struct tm has the tm_gmtoff member.])],
     ,
     [#include <time.h>])

# test for boost anyways
BOOST_REQUIRE([1.36], [boost=no])
if test "$boost" == ""; then
  BOOST_TEST
fi

AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen], [true])

AC_CONFIG_FILES([Makefile yahttp/Makefile tests/Makefile examples/Makefile docs/Makefile docs/yahttp.cfg fuzzing/Makefile])
AC_CONFIG_LINKS([tests/request-chunked.txt:tests/request-chunked.txt
tests/request-get-cookies-ok.txt:tests/request-get-cookies-ok.txt
tests/request-get-incomplete.txt:tests/request-get-incomplete.txt
tests/request-get-ok.txt:tests/request-get-ok.txt
tests/request-issue-7.txt:tests/request-issue-7.txt
tests/request-post-ok.txt:tests/request-post-ok.txt
tests/response-binary.txt:tests/response-binary.txt
tests/response-chunked-body.txt:tests/response-chunked-body.txt
tests/response-chunked-headers.txt:tests/response-chunked-headers.txt
tests/response-google-200.txt:tests/response-google-200.txt
tests/response-incomplete.txt:tests/response-incomplete.txt])

AC_MSG_CHECKING([whether to enable fuzzing])
AC_ARG_ENABLE([fuzzing],
  AS_HELP_STRING([--enable-fuzzing],
    [enable fuzzing @<:@default=no@:>@]),
  [enable_fuzzing=$enableval],
  [enable_fuzzing=no]
)
AC_MSG_RESULT([$enable_fuzzing])
AM_CONDITIONAL([FUZZING], [test "x$enable_fuzzing" != "xno"])

AC_OUTPUT
