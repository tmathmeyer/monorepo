langs("C")

cpp_header (
  name = "json_headers",
  srcs = [
    "json.h",
    "json_io.h",
    "json_rectify.h",
  ],
)

cpp_object (
  name = "json",
  srcs = [
    "json.cc",
  ],
  deps = [
    ":json_headers",
  ],
)

cpp_object (
  name = "json_io",
  srcs = [
    "json_io.cc",
  ],
  deps = [
    ":json",
    ":json_headers",
  ],
)
