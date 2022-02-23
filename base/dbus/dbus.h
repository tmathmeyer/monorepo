
#ifndef BASE_DBUS_DBUS_H_
#define BASE_DBUS_DBUS_H_

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <dbus/dbus.h>

#include "base/tuple.h"
#include "base/json/json.h"
#include "base/json/json_rectify.h"
#include "base/dbus/dbus_parser.h"


namespace base {
namespace dbus {

// Helper namespace for unpacking c++ arg syntax into DBUS arg syntax
namespace {

template<typename T> struct DBusType { };
template<> struct DBusType<char const*> {
  static const char Value = 's';
};

template<typename... T>
bool AppendArgs(T... t, DBusMessage* msg) {
  return dbus_message_append_args(msg, t..., DBUS_TYPE_INVALID);
}

template<typename F, typename... U, typename... T>
bool AppendArgs(T... t, DBusMessage* msg, F f, U... u) {
  return AppendArgs<char, F*, T..., U...>(
    DBusType<F>::Value, &f, msg, u...);
}

}  // namespace

// Forward declare
class ObjectProxy;

// The underlying connection object for which all proxies ultimately talk to.
class Connection : public std::enable_shared_from_this<Connection> {
 public:
  static std::shared_ptr<Connection> GetSystemConnection();

  template<typename T>
  typename std::unique_ptr<T> GetInterface(std::string ns, std::string path) {
    ObjectProxy proxy(shared_from_this(), ns, path);
    return T::CreateFromProxy(proxy);
  }


 private:
  friend class ObjectProxy;

  // Private constructor - only access via static initializer.
  explicit Connection(DBusConnection* connection);

  // Calls a method
  template<typename ... Args>
  base::json::JSON CallMethod(std::string ns,
                              std::string obj,
                              std::string iface,
                              std::string method,
                              Args ... args) {
    DBusMessage *msg = dbus_message_new_method_call(
    ns.c_str(), obj.c_str(), iface.c_str(), method.c_str());
    if (!msg)
      return {};
    if (!AppendArgs(msg, args...))
      return {};
    DBusPendingCall *pending;
    if (!dbus_connection_send_with_reply(connection_, msg, &pending, -1))
      return {};
    if (!pending)
      return {};
    dbus_connection_flush(connection_);
    dbus_message_unref(msg);
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if (!msg)
      return {};
    dbus_pending_call_unref(pending);
    auto result = DecodeMessageReply(msg);
    dbus_message_unref(msg);
    return result;
  }

  DBusConnection *connection_;
};

namespace {

template<typename T>
using Name = std::string;

template<typename T>
struct Types {
  using Json = std::string;
  using Proxy = T;
};

template<>
struct Types<std::string> {
  using Json = std::string;
  using Proxy = Json;
};

template<>
struct Types<bool> {
  using Json = bool;
  using Proxy = Json;
};

template<>
struct Types<ssize_t> {
  using Json = ssize_t;
  using Proxy = Json;
};

template<>
struct Types<double> {
  using Json = double;
  using Proxy = Json;
};

template<typename ...T>
struct Binder {};

template<typename F, typename... R>
struct Binder<F, R...> {
  static std::tuple<typename Types<F>::Proxy,
                    typename Types<R>::Proxy...> Bind(
      std::shared_ptr<Connection> conn, std::string ns,
      std::tuple<typename Types<F>::Json,
                 typename Types<R>::Json...> pack) {
    auto first = std::get<0>(pack);
    auto rest = base::MetaTuple<typename Types<F>::Json,
                                typename Types<R>::Json...>::Rest(
                                  std::move(pack));
    auto recurse = Binder<R...>::Bind(conn, ns, std::move(rest));
    if constexpr (std::is_same_v<F, typename Types<F>::Json>) {
      return std::tuple_cat(
        std::make_tuple<typename Types<F>::Proxy>(std::move(first)),
        std::move(recurse));
    } else {
      auto storage =
        std::make_tuple<typename Types<F>::Proxy>(
          std::make_unique<ObjectProxy>(conn, ns, std::move(first)));
      return std::tuple_cat(
        std::move(storage),
        std::move(recurse));
    }
  }
};

template<>
struct Binder<> {
  static std::tuple<> Bind(std::shared_ptr<Connection> conn,
                           std::string ns, std::tuple<> pack) {
    return pack;
  }
};

template<typename T, typename... Args>
std::unique_ptr<T> Instantiate(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

}  // namespace


class ObjectProxy {
 public:
  using Storage = std::unique_ptr<ObjectProxy>;

  ObjectProxy(std::shared_ptr<Connection> conn, std::string ns,
              std::string path);

  template<typename T, typename... Args>
  std::unique_ptr<T> Create(Name<Args>... args) const {
    base::json::JSON properties = connection_->CallMethod(
      ns_, path_, "org.freedesktop.DBus.Properties", "GetAll",
      T::GetTypeName());

    if (base::json::IsNull(properties))
      return nullptr;

    auto object = base::json::Unpack<base::json::Object>(std::move(properties));
    if (!object)
      return nullptr;

    auto rectified = base::json::Rectify<typename Types<Args>::Json...>(
      *object, args...);

    if (!rectified.has_value())
      return nullptr;

    std::tuple<Args...> bound = Binder<Args...>::Bind(
      connection_, ns_, std::move(rectified).value());

    using SelfType = std::unique_ptr<ObjectProxy>;
    auto tup = std::make_tuple<SelfType>(
      std::make_unique<ObjectProxy>(connection_, ns_, path_));

    return std::apply(
      Instantiate<T, SelfType, Args...>,
      std::tuple_cat(std::move(tup), std::move(bound)));
  }

 private:
  friend class Connection;

  std::shared_ptr<Connection> connection_;
  std::string ns_;
  std::string path_;
};

}  // namespace dbus
}  // namespace base


std::ostream& operator<<(std::ostream& os, const std::unique_ptr<base::dbus::ObjectProxy>& pr);

#endif  // ifndef BASE_DBUS_DBUS_H_