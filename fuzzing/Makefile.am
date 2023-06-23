if FUZZING

LIB_FUZZING_ENGINE ?= standalone_fuzz_target_runner.o

standalone_fuzz_target_runner.o: standalone_fuzz_target_runner.cc

bin_PROGRAMS = fuzz_yahttp
fuzz_yahttp_SOURCES = fuzz_yahttp.cc
fuzz_yahttp_CFLAGS = $(RELRO_CFLASG) $(PIE_CFLAGS) -I$(top_srcdir)
fuzz_yahttp_CXXFLAGS = $(RELRO_CFLASG) $(PIE_CFLAGS) -I$(top_srcdir)
fuzz_yahttp_LDADD = $(RELRO_LDFLAGS) $(PIE_LDFLAGS) ../yahttp/libyahttp.la $(LIB_FUZZING_ENGINE)
# we need the mockup runner to be built, but not linked if a real fuzzing engine is used
fuzz_yahttp_DEPENDENCIES = standalone_fuzz_target_runner.o
endif
