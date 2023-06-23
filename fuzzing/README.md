Fuzzing the YaHTTP library
--------------------------

This repository contains one fuzzing target that can be used with generic
fuzzing engines like AFL and libFuzzer.
The target is built by passing the --enable-fuzzing option to the
configure, then building as usual.

By default the fuzzing target is linked against a standalone target,
standalone_fuzz_target_runner.cc, which does no fuzzing but makes it easy
to check a given test file, or just that the fuzzing target can be built properly.

This behaviour can be changed via the LIB_FUZZING_ENGINE variable, for example
by setting it to -lFuzzer, building with clang by setting CC=clang CXX=clang++
before running the configure and adding '-fsanitize=fuzzer-no-link' to CFLAGS
and CXXFLAGS. Doing so instructs the compiler to instrument the code for
efficient fuzzing but not to link directly with -lFuzzer, which would make
the compilation tests done during the configure phase fail.

Sanitizers
----------

In order to catch the maximum of issues during fuzzing, it makes sense to
enable the ASAN and UBSAN sanitizers by setting CXXFLAGS="-fsanitize=address
-fsanitize=undefined" during the configure.

Corpus
------

This directory contains a few files used for continuous fuzzing in the
'fuzzing/corpus' directory.

Quickly getting started (using clang 11)
----------------------------------------
First, confgure:

```
LIB_FUZZING_ENGINE="/usr/lib/clang/11.0.1/lib/linux/libclang_rt.fuzzer-x86_64.a" \
  CC=clang \
  CXX=clang++ \
  CFLAGS='-fsanitize=fuzzer-no-link -fsanitize=address -fsanitize=undefined' \
  CXXFLAGS='-fsanitize=fuzzer-no-link -fsanitize=address -fsanitize=undefined' \
  ./configure --enable-fuzzing
```

Then build:

```
LIB_FUZZING_ENGINE="/usr/lib/clang/11.0.1/lib/linux/libclang_rt.fuzzer-x86_64.a" \
  make
```

Now you're ready to run one the fuzzing target.
First, copy the starting corpus:

```
mkdir new-corpus
./fuzzing/fuzz_target_yahttp -merge=1 new-corpus fuzzing/corpus/YYYYY
```

Then run the thing:
```
./fuzzing/fuzz_target_yahttp new-corpus
```

The [LLVM docs](https://llvm.org/docs/LibFuzzer.html) have more info.
