
#include <dbus/dbus.h>

#include "base/dbus/dbus.h"

namespace base {
namespace dbus {

// static
std::shared_ptr<Connection> Connection::GetSystemConnection() {
  DBusError err;
  dbus_error_init(&err);
  DBusConnection *connection = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
  if (dbus_error_is_set(&err)) {
    fprintf(stderr, "Connection Error (%s)\n", err.message);
    dbus_error_free(&err);
    return nullptr;
  }
  std::shared_ptr<Connection> conn(new Connection(connection));
  return conn;
}

Connection::Connection(DBusConnection* connection) : connection_(connection) {}


ObjectProxy::ObjectProxy(std::shared_ptr<Connection> conn, std::string ns,
                         std::string path)
    : connection_(std::move(conn)),
      ns_(ns),
      path_(path) {}

}  // namespace dbus
}  // namespace base

std::ostream& operator<<(std::ostream& os, const std::unique_ptr<base::dbus::ObjectProxy>& pr) {
  os << "[ObjectProxy]";
  return os;
}