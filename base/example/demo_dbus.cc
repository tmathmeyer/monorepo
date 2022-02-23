
#include <iostream>

#include "base/dbus/dbus.h"
#include "base/json/json_io.h"

class Adapter {
 public:
  static const char* GetTypeName() {
    return "net.connman.iwd.Adapter";
  }

  static std::unique_ptr<Adapter> CreateFromProxy(
      const base::dbus::ObjectProxy& ns) {
    return ns.Create<Adapter, std::string, std::string, std::string>(
        "Model", "Name", "Vendor");
  }

  Adapter(std::unique_ptr<base::dbus::ObjectProxy> self,
          std::string model, std::string name, std::string vendor) {
    std::cout << "model = " << model << "\n";
    std::cout << "name = " << name << "\n";
    std::cout << "vendor = " << vendor << "\n";
  }
};

class Device {
 public:
  static const char* GetTypeName() {
    return "net.connman.iwd.Device";
  }

  static std::unique_ptr<Device> CreateFromProxy(
      const base::dbus::ObjectProxy& ns) {
    return ns.Create<Device, std::string, std::string, bool,
      typename base::dbus::ObjectProxy::Storage>(
        "Address", "Name", "Powered", "Adapter");
  }

  Device(std::unique_ptr<base::dbus::ObjectProxy> self,
         std::string address, std::string name, bool powered,
         std::unique_ptr<base::dbus::ObjectProxy> adapter) {
    address_ = address;
    name_ = name;
    powered_ = powered;
    adapter_ = std::move(adapter);
  }

  const std::string& Address() { return address_; }
  const std::string& Name() { return name_; }
  bool Powered() { return powered_; }

  std::unique_ptr<Adapter> GetAdapter() {
    return Adapter::CreateFromProxy(*adapter_);
  }

 private:
  friend class base::dbus::ObjectProxy;

  std::string address_;
  std::string name_;
  bool powered_;
  std::unique_ptr<base::dbus::ObjectProxy> adapter_;
};






int main() {
  auto conn = base::dbus::Connection::GetSystemConnection();
  if (!conn) {
    puts("No Connection");
    return -1;
  }

  std::unique_ptr<Device> dev = conn->GetInterface<Device>(
    "net.connman.iwd", "/net/connman/iwd/0/4");

  std::cout << "Address: " << dev->Address() << "\n";
  dev->GetAdapter();
}