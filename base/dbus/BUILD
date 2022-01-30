langs("C")

cpp_header (
  name = "dbus_h",
  srcs = [ "dbus.h" ],
  deps = [
    "//base:json_h",
  ],
)

cpp_object (
  name = "dbus",
  srcs = [ "dbus.cc", ],
  deps = [
    ":dbus_h",
    "//base:json_h",
    "//base:check_h",
    "//base:json",
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