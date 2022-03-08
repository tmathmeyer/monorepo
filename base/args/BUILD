load("//rules/core/C/build_defs.py")

c_header (
  name = "argparse",
  srcs = [ "argparse.h" ],
)

cpp_binary (
  name = "argparse_test",
  srcs = [ "test.cpp" ],
  deps = [
    ":argparse",
    "//googletest:googletest",
    "//googletest:googletest_headers",
  ],
  include_dirs = [
    "googletest/googletest/include",
    "googletest/googletest",
  ],
  flags = [ "-lpthread" ],
)