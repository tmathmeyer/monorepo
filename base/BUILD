langs("C")

cpp_header (
  name = "location_h",
  srcs = [ "location.h" ],
)

cpp_header (
  name = "check_h",
  srcs = [ "check.h" ],
)

cpp_header (
  name = "json_h",
  srcs = [ "json.h" ],
  deps = [ ":check_h", ],
)

cpp_header (
  name = "serialize_base_h",
  srcs = [
    "serialize_base.h",
  ],
  deps = [ ":json_h", ],
)

cpp_header (
  name = "serialize_h",
  srcs = [
    "serialize.h",
  ],
  deps = [
    ":serialize_base_h",
    ":json_h",
  ],
)

cpp_header (
  name = "status_h",
  srcs = [ "status.h" ],
  deps = [
    ":location_h",
    ":serialize_base_h",
  ],
)

cpp_object (
  name = "json",
  srcs = [ "json.cc", ],
  deps = [
    ":json_h",
    ":check_h",
  ],
)

cpp_object (
  name = "status",
  srcs = [ "status.cc", ],
  deps = [
    ":json_h",
    ":status_h",
    ":serialize_h",
    ":json",
  ],
)

cpp_header (
  name = "bind",
  srcs = [
    "bind.h",
  ],
  deps = [
    ":check_h",
    ":location_h",
  ],
)