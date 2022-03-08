langs("C")

cpp_object (
  name = "tracing",
  srcs = [ "trace.cc" ],
  deps = [
    ":trace_h",
    "//base/json:json_headers",
    "//base/json:json_io",
    "//base/json:json",
  ],
)

cpp_header (
  name = "trace_h",
  srcs = [ "trace.h" ],
)