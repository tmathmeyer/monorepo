
#include "base/dbus/dbus.h"

int main() {
  auto conn = base::dbus::Connection::GetSystemConnection();
  if (conn.has_value()) {
    auto value = conn.value()
      ->GetNamespace("net.connman.iwd")
      ->GetObject("/")
      ->GetInterface("org.freedesktop.DBus.ObjectManager")
      ->CallMethod("GetManagedObjects");

      
    
  } else {
    puts("no Connection");
  }
}