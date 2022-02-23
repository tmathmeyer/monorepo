langs("C")

cpp_binary (
  name = "demo_bind",
  srcs = [ "demo_bind.cc" ],
  deps = [ "//base:bind" ],
)

cpp_binary (
  name = "demo_dbus",
  srcs = [ "demo_dbus.cc" ],
  deps = [
    "//base/dbus:dbus",
    "//base/dbus:dbus_headers",
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