langs("C")

cpp_header (
  name = "dbus_headers",
  srcs = [
    "dbus.h",
    "dbus_parser.h",
  ],
  deps = [
    "//base:tuple_h",
    "//base/json:json_headers",
  ],
)

cpp_object (
  name = "dbus",
  srcs = [
    "dbus.cc",
  ],
  deps = [
    ":dbus_headers",
    ":dbus_parser",
    "//base/json:json",
    "//base/json:json_io",
  ],
  include_dirs = [
    "/usr/lib/dbus-1.0/include",
    "/usr/include/dbus-1.0",
    "/usr/include/glib-2.0",
    "/usr/lib/glib-2.0/include",
  ],
  flags = [
    "-ldbus-glib-1",
    "-ldbus-1",
    "-lglib-2.0",
  ],
)

cpp_object (
  name = "dbus_parser",
  srcs = [
    "dbus_parser.cc",
  ],
  deps = [
    ":dbus_headers",
    "//base/json:json",
    "//base/json:json_io",
  ],
  include_dirs = [
    "/usr/lib/dbus-1.0/include",
    "/usr/include/dbus-1.0",
    "/usr/include/glib-2.0",
    "/usr/lib/glib-2.0/include",
  ],
  flags = [
    "-ldbus-glib-1",
    "-ldbus-1",
    "-lglib-2.0",
  ],
)