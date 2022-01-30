
#include <dbus/dbus.h>

#include "base/dbus/dbus.h"
#include "base/check.h"
#include "base/json.h"

namespace base {
namespace dbus {

namespace {

// forward declare.
std::vector<base::Value> Iter2Array(DBusMessageIter *args);
base::Value UnpackVariant(DBusMessageIter *args);

base::Value DecodeType(int type, DBusMessageIter *args) {
  switch(type) {
    case DBUS_TYPE_STRING: /* s */
    case DBUS_TYPE_OBJECT_PATH: /* o */{
      char *str = nullptr;
      dbus_message_iter_get_basic(args, &str);
      return base::Value(str);
    }
    case DBUS_TYPE_ARRAY: /* a */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      return base::Value(Iter2Array(&subtype));
    }
    case DBUS_TYPE_STRUCT: /* r */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      return base::Value(Iter2Array(&subtype));
    }
    case DBUS_TYPE_DICT_ENTRY: /* e */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      auto vec = Iter2Array(&subtype);
      std::map<std::string, base::Value> unwinder;
      std::optional<std::string> key = std::nullopt;
      for (const auto& v : vec) {
        if (!key.has_value()) {
          key = v.AsString();
        } else {
          if (v.type() == Value::Type::kArray)
            unwinder.insert({key.value(), v[0]});
          else
            unwinder.insert({key.value(), v});
          key = std::nullopt;
        }
      }
      CHECK(key == std::nullopt);
      return base::Value(std::move(unwinder));
    }
    case DBUS_TYPE_VARIANT: /* v */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      return UnpackVariant(&subtype);
    }
    case DBUS_TYPE_BOOLEAN: /* b */{
      bool value;
      dbus_message_iter_get_basic(args, &value);
      return base::Value(value);
    }
    case DBUS_TYPE_UINT16: /* q */{
      uint16_t value;
      dbus_message_iter_get_basic(args, &value);
      return base::Value(value);
    }
    case DBUS_TYPE_INT16: /* q */{
      int16_t value;
      dbus_message_iter_get_basic(args, &value);
      return base::Value(value);
    }
  }

  printf("type = %c\n", type);
  NOTREACHED();

  return base::Value();
}

base::Value UnpackVariant(DBusMessageIter *args) {
  char *sig = dbus_message_iter_get_signature(args);
  return DecodeType(sig[0], args);
}

std::vector<base::Value> Iter2Array(DBusMessageIter *args) {
  int type;
  std::vector<base::Value> result;
  while ((type = dbus_message_iter_get_arg_type(args)) != DBUS_TYPE_INVALID) {
    result.push_back(DecodeType(type, args));
    dbus_message_iter_next(args);
  }
  return result;
}

std::optional<base::Value> DecodeMessageReply(DBusMessage *msg) {
  DBusMessageIter args;

  if (!dbus_message_iter_init(msg, &args))
    return std::nullopt;
  
  std::vector<base::Value> result = Iter2Array(&args);

  if (result.size() == 1)
    return result[0];

  return base::Value(std::move(result));
}


}  // namespace

// static
std::optional<std::shared_ptr<Connection>> Connection::GetSystemConnection() {
  DBusError err;
  dbus_error_init(&err);
  DBusConnection *connection = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
  if (dbus_error_is_set(&err)) {
    fprintf(stderr, "Connection Error (%s)\n", err.message);
    dbus_error_free(&err);
    return std::nullopt;
  }
  std::shared_ptr<Connection> conn(new Connection(connection));
  return conn;
}

Connection::Connection(DBusConnection* connection) : connection_(connection) {}

std::map<std::string, std::shared_ptr<Namespace>> Connection::GetAllNamespaces() {
  puts("Query All Namespaces");
  std::map<std::string, std::shared_ptr<Namespace>> result;
  std::shared_ptr<Namespace> dbns(
    new Namespace("org.freedesktop.DBus", shared_from_this()));
  auto object = dbns->GetObject("/");
  auto interface = object->GetInterface("org.freedesktop.DBus");
  auto names = interface->CallMethod("ListNames");
  if (!names.has_value())
    return result;
  return result;
}

std::shared_ptr<Namespace> Connection::GetNamespace(std::string name) {
  std::shared_ptr<Namespace> ns(new Namespace(name, shared_from_this()));
  return ns;
}

std::optional<base::Value> Connection::CallMethod(
    std::string ns, std::string obj, std::string iface, std::string method) {
  DBusMessage *msg = dbus_message_new_method_call(
    ns.c_str(), obj.c_str(), iface.c_str(), method.c_str());
  if (!msg)
    return std::nullopt;
  DBusPendingCall *pending;
  if (!dbus_connection_send_with_reply(connection_, msg, &pending, -1))
    return std::nullopt;
  if (!pending)
    return std::nullopt;
  dbus_connection_flush(connection_);
  dbus_message_unref(msg);
  dbus_pending_call_block(pending);
  msg = dbus_pending_call_steal_reply(pending);
  if (!msg)
    return std::nullopt;
  dbus_pending_call_unref(pending);
  auto result = DecodeMessageReply(msg);
  dbus_message_unref(msg);
  return result;
}

Namespace::Namespace(std::string name, std::shared_ptr<Connection> connection)
    : name_(name),
      connection_(std::move(connection)) {}

std::map<std::string, std::shared_ptr<Object>> Namespace::GetAllObjects() {
  puts("Query All Objects");
  std::map<std::string, std::shared_ptr<Object>> result;
  std::shared_ptr<Object> root(
    new Object("/", shared_from_this()));
  auto spec = root->GetInterface("org.freedesktop.DBus.Introspectable");
  auto ispect = spec->CallMethod("Introspect");
  std::cout << ispect.value() << "\n";
  return result;
}

std::shared_ptr<Object> Namespace::GetObject(std::string path) {
  std::shared_ptr<Object> obj(new Object(path, shared_from_this()));
  return obj;
}

std::optional<base::Value> Namespace::CallMethod(
    std::string obj, std::string iface, std::string method) {
  return connection_->CallMethod(name_, obj, iface, method);
}

Object::Object(std::string path, std::shared_ptr<Namespace> ns)
    : path_(path),
      namespace_(std::move(ns)) {}

// Gets all the interfaces available.
std::map<std::string, std::shared_ptr<Interface>> Object::GetAllInterfaces() {
  std::map<std::string, std::shared_ptr<Interface>> result;
  NOTREACHED();
  return result;
}

// Gets a interface by name, or nullopt if it doesn't exist
std::shared_ptr<Interface> Object::GetInterface(std::string name) {
  std::shared_ptr<Interface> iface(new Interface(name, shared_from_this()));
  return iface;
}

std::optional<base::Value> Object::CallMethod(std::string iface, std::string method) {
  return namespace_->CallMethod(path_, iface, method);
}

Interface::Interface(std::string name, std::shared_ptr<Object> object)
    : name_(name),
      object_(std::move(object)) {}

std::optional<base::Value> Interface::CallMethod(std::string method) {
  return object_->CallMethod(name_, method);
}


}  // namespace dbus
}  // namespace base