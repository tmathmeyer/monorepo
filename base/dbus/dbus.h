
#ifndef BASE_DBUS_DBUS_H_
#define BASE_DBUS_DBUS_H_

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <dbus/dbus.h>

#include "base/json.h"

namespace base {
namespace dbus {

class Namespace;
class Object;
class Interface;

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  static std::optional<std::shared_ptr<Connection>> GetSystemConnection();

  // Gets all the namespaces available.
  std::map<std::string, std::shared_ptr<Namespace>> GetAllNamespaces();

  // Gets a namespace by name
  std::shared_ptr<Namespace> GetNamespace(std::string name);

 private:
  friend class Namespace;

  // Private constructor - only access via static initializer.
  explicit Connection(DBusConnection* connection);

  std::optional<base::Value> CallMethod(
    std::string ns, std::string obj, std::string iface, std::string method);

  DBusConnection *connection_;
};


class Namespace : public std::enable_shared_from_this<Namespace> {
 public:
  std::string GetName() const { return name_; }

  // Gets all the objects available.
  std::map<std::string, std::shared_ptr<Object>> GetAllObjects();

  // Gets a object by path
  std::shared_ptr<Object> GetObject(std::string path);

 private:
  friend class Connection;
  friend class Object;

  std::optional<base::Value> CallMethod(
    std::string obj, std::string iface, std::string method);

  // Private constructor accessable from Connection only
  Namespace(std::string name, std::shared_ptr<Connection>);

  std::string name_;
  std::shared_ptr<Connection> connection_;
};


class Object : public std::enable_shared_from_this<Object> {
 public:
  std::string GetPath() { return path_; }

  // Gets all the interfaces available.
  std::map<std::string, std::shared_ptr<Interface>> GetAllInterfaces();

  // Gets a interface by name
  std::shared_ptr<Interface> GetInterface(std::string name);

 private:
  friend class Namespace;
  friend class Interface;

  std::optional<base::Value> CallMethod(std::string iface, std::string method);

  Object(std::string path, std::shared_ptr<Namespace>);

  std::string path_;
  std::shared_ptr<Namespace> namespace_;
};

class Interface : std::enable_shared_from_this<Interface> {
 public:
  std::string GetName() { return name_; };

  std::optional<base::Value> CallMethod(std::string method);

 private:
  friend class Object;

  Interface(std::string name, std::shared_ptr<Object>);

  std::string name_;
  std::shared_ptr<Object> object_;
};

class Method {

};

class Property {

};

class Signal {

};

}  // namespace dbus
}  // namespace base

#endif  // ifndef BASE_DBUS_DBUS_H_