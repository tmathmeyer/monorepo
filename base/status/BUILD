langs("C")

cpp_header (
  name = "status_h",
  srcs = [ "status.h" ],
  deps = [
    "//base:check",
    "//base:location",
  ],
)

cpp_object (
  name = "status",
  srcs = ["status.cc" ],
  deps = [
    ":status_h",
  ],
)
