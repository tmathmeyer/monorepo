
#ifndef BASE_DBUS_DBUS_H_
#define BASE_DBUS_DBUS_H_

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <dbus/dbus.h>

#include "base/dbus/dbus_parser.h"
#include "base/json/json.h"
#include "base/json/json_io.h"
#include "base/json/json_rectify.h"
#include "base/tuple.h"

namespace base {
namespace dbus {

// Helper namespace for unpacking c++ arg syntax into DBUS arg syntax
namespace {

template <typename T>
struct DBusType {};
template <>
struct DBusType<char const*> {
  static const char Value = 's';
};

template <typename... T>
bool AppendArgs(T... t, DBusMessage* msg) {
  return dbus_message_append_args(msg, t..., DBUS_TYPE_INVALID);
}

template <typename F, typename... U, typename... T>
bool AppendArgs(T... t, DBusMessage* msg, F f, U... u) {
  return AppendArgs<char, F*, T..., U...>(DBusType<F>::Value, &f, msg, u...);
}

}  // namespace

// Forward declare
class ObjectProxy;

// The underlying connection object for which all proxies ultimately talk to.
class Connection : public std::enable_shared_from_this<Connection> {
 public:
  static std::shared_ptr<Connection> GetSystemConnection();

  template <typename T>
  typename std::unique_ptr<T> GetInterface(std::string ns, std::string path) {
    ObjectProxy proxy(shared_from_this(), ns, path, std::nullopt);
    return T::CreateFromProxy(proxy);
  }

 private:
  friend class ObjectProxy;

  // Private constructor - only access via static initializer.
  explicit Connection(DBusConnection* connection);

  // Calls a method
  template <typename... Args>
  base::json::JSON CallMethod(std::string ns,
                              std::string obj,
                              std::string iface,
                              std::string method,
                              Args... args) {
    DBusMessage* msg = dbus_message_new_method_call(
        ns.c_str(), obj.c_str(), iface.c_str(), method.c_str());
    if (!msg)
      return {};
    if (!AppendArgs(msg, args...))
      return {};
    DBusPendingCall* pending;
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

  template <typename... Args>
  void CallVoidMethod(std::string ns,
                      std::string obj,
                      std::string iface,
                      std::string method,
                      Args... args) {
    DBusMessage* msg = dbus_message_new_method_call(
        ns.c_str(), obj.c_str(), iface.c_str(), method.c_str());
    if (!msg)
      return;
    if (!AppendArgs(msg, args...))
      return;
    DBusError err;
    dbus_error_init(&err);
    dbus_connection_send_with_reply_and_block(connection_, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
      fprintf(stderr, "Connection Error (%s)\n", err.message);
      dbus_error_free(&err);
      return;
    }
    dbus_connection_flush(connection_);
    dbus_message_unref(msg);
  }

  DBusConnection* connection_;
};

namespace {

template <typename T>
using Name = std::string;

template <typename T>
struct Types {
  using Json = std::string;
  using Proxy = T;
  using Type = T;
};

template <typename T>
struct Types<std::optional<T>> {
  using Json = std::optional<std::string>;
  using Proxy = std::optional<T>;
  using Type = T;
};

template <>
struct Types<std::string> {
  using Json = std::string;
  using Proxy = Json;
};

template <>
struct Types<bool> {
  using Json = bool;
  using Proxy = Json;
};

template <>
struct Types<ssize_t> {
  using Json = ssize_t;
  using Proxy = Json;
};

template <>
struct Types<double> {
  using Json = double;
  using Proxy = Json;
};

template <typename... T>
struct Binder {};

template <typename F, typename... R>
struct Binder<F, R...> {
  static std::tuple<typename Types<F>::Proxy, typename Types<R>::Proxy...> Bind(
      std::shared_ptr<Connection> conn,
      std::string ns,
      std::tuple<typename Types<F>::Json, typename Types<R>::Json...> pack) {
    auto first = std::get<0>(pack);
    auto rest =
        base::MetaTuple<typename Types<F>::Json,
                        typename Types<R>::Json...>::Rest(std::move(pack));
    auto recurse = Binder<R...>::Bind(conn, ns, std::move(rest));
    if constexpr (std::is_same_v<F, typename Types<F>::Json>) {
      return std::tuple_cat(
          std::make_tuple<typename Types<F>::Proxy>(std::move(first)),
          std::move(recurse));
    } else if constexpr (std::is_same_v<F, typename Types<F>::Type>) {
      // This is for binding a string into a _required_ proxy to generate
      // objects from.
      std::tuple<typename Types<F>::Proxy> storage =
          std::make_tuple<typename Types<F>::Proxy>(
              std::make_unique<ObjectProxy>(conn, ns, std::move(first),
                                            std::nullopt));
      return std::tuple_cat(std::move(storage), std::move(recurse));
    } else {
      // This is for binding a ?string into a ?object proxy
      std::tuple<typename Types<F>::Proxy> storage;
      if (first.has_value()) {
        storage = std::make_tuple<typename Types<F>::Proxy>(
            std::make_unique<ObjectProxy>(conn, ns, std::move(first).value(),
                                          std::nullopt));
      } else {
        storage = std::make_tuple<typename Types<F>::Proxy>(std::nullopt);
      }
      return std::tuple_cat(std::move(storage), std::move(recurse));
    }
  }
};

template <>
struct Binder<> {
  static std::tuple<> Bind(std::shared_ptr<Connection> conn,
                           std::string ns,
                           std::tuple<> pack) {
    return pack;
  }
};

template <typename T, typename... Args>
std::unique_ptr<T> Instantiate(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

struct ProxyPack {
  std::shared_ptr<Connection> connection;
  std::string ns;
};

template <typename T>
struct ProxyDeconstructor {
  using RawType = std::string;
  static std::optional<T> Reconstruct(const ProxyPack& src, RawType&& input) {
    ObjectProxy replaced(src.connection, src.ns, input, std::nullopt);
    return T::element_type::CreateFromProxy(replaced);
  }
};

template <>
struct ProxyDeconstructor<std::tuple<>> {
  using RawType = std::tuple<>;
  static std::optional<std::tuple<>> Reconstruct(const ProxyPack& src,
                                                 RawType&& input) {
    return std::move(input);
  }
};

template <typename F, typename... T>
struct ProxyDeconstructor<std::tuple<F, T...>> {
  using RawType = std::tuple<typename ProxyDeconstructor<F>::RawType,
                             typename ProxyDeconstructor<T>::RawType...>;
  static std::optional<std::tuple<F, T...>> Reconstruct(const ProxyPack& src,
                                                        RawType&& input) {
    auto first =
        ProxyDeconstructor<F>::Reconstruct(src, std::move(std::get<0>(input)));
    if (!first.has_value())
      return std::nullopt;

    auto rest = ProxyDeconstructor<std::tuple<T...>>::Reconstruct(
        src, MetaTuple<typename ProxyDeconstructor<F>::RawType,
                       typename ProxyDeconstructor<T>::RawType...>::
                 Rest(std::move(input)));
    if (!rest.has_value())
      return std::nullopt;

    return std::tuple_cat(std::make_tuple<F>(std::move(first).value()),
                          std::move(rest).value());
  }
};

template <typename T>
struct ProxyDeconstructor<std::vector<T>> {
  using RawType = std::vector<typename ProxyDeconstructor<T>::RawType>;
  static std::optional<std::vector<T>> Reconstruct(const ProxyPack& src,
                                                   RawType&& input) {
    std::vector<T> result;
    for (auto& raw : input) {
      std::optional<T> recon =
          ProxyDeconstructor<T>::Reconstruct(src, std::move(raw));
      if (!recon.has_value())
        return std::nullopt;
      result.push_back(std::move(recon).value());
    }
    return result;
  }
};

template <>
struct ProxyDeconstructor<base::json::Number> {
  using RawType = base::json::Number;
  static std::optional<RawType> Reconstruct(const ProxyPack&, RawType&& input) {
    return std::move(input);
  }
};

template <>
struct ProxyDeconstructor<base::json::Float> {
  using RawType = base::json::Float;
  static std::optional<RawType> Reconstruct(const ProxyPack&, RawType&& input) {
    return std::move(input);
  }
};

template <>
struct ProxyDeconstructor<bool> {
  using RawType = bool;
  static std::optional<RawType> Reconstruct(const ProxyPack&, RawType&& input) {
    return std::move(input);
  }
};

template <>
struct ProxyDeconstructor<std::string> {
  using RawType = std::string;
  static std::optional<RawType> Reconstruct(const ProxyPack&, RawType&& input) {
    return std::move(input);
  }
};

}  // namespace

class ObjectProxy {
 public:
  using Storage = std::unique_ptr<ObjectProxy>;
  using MaybeStorage = std::optional<Storage>;

  ObjectProxy(std::shared_ptr<Connection> conn,
              std::string ns,
              std::string path,
              std::optional<std::string> iface);

  template <typename T, typename... Args>
  std::unique_ptr<T> Create(Name<Args>... args) const {
    base::json::JSON properties =
        connection_->CallMethod(ns_, path_, "org.freedesktop.DBus.Properties",
                                "GetAll", T::GetTypeName());

    if (base::json::IsNull(properties))
      return nullptr;

    auto object = base::json::Unpack<base::json::Object>(std::move(properties));
    if (!object)
      return nullptr;

    auto rectified =
        base::json::Rectify<typename Types<Args>::Json...>(*object, args...);

    if (!rectified.has_value()) {
      std::cout << "Failed to Create " << T::GetTypeName()
                << ", args are: " << *object << "\n";
      return nullptr;
    }

    std::tuple<Args...> bound =
        Binder<Args...>::Bind(connection_, ns_, std::move(rectified).value());

    using SelfType = std::unique_ptr<ObjectProxy>;
    auto tup = std::make_tuple<SelfType>(std::make_unique<ObjectProxy>(
        connection_, ns_, path_, T::GetTypeName()));

    return std::apply(Instantiate<T, SelfType, Args...>,
                      std::tuple_cat(std::move(tup), std::move(bound)));
  }

  template <typename Ret, typename... Args>
  std::optional<Ret> Call(std::string method, Args&&... args) {
    if (!iface_.has_value())
      return std::nullopt;

    base::json::JSON expr = connection_->CallMethod<Args...>(
        ns_, path_, *iface_, method, std::forward<Args>(args)...);

    using RawRet = typename ProxyDeconstructor<Ret>::RawType;

    std::optional<RawRet> parsed =
        base::json::Parser<RawRet>::Parse(std::move(expr));

    if (!parsed.has_value())
      return std::nullopt;

    ProxyPack proxy_pack;
    proxy_pack.connection = connection_;
    proxy_pack.ns = ns_;
    return ProxyDeconstructor<Ret>::Reconstruct(proxy_pack,
                                                std::move(parsed).value());
  }

  template <typename... Args>
  void Call(std::string method, Args&&... args) {
    if (iface_.has_value())
      connection_->CallVoidMethod<Args...>(ns_, path_, *iface_, method,
                                           std::forward<Args>(args)...);
  }

 private:
  friend class Connection;

  std::shared_ptr<Connection> connection_;
  std::string ns_;
  std::string path_;
  std::optional<std::string> iface_;
};

}  // namespace dbus
}  // namespace base

std::ostream& operator<<(std::ostream& os,
                         const std::unique_ptr<base::dbus::ObjectProxy>& pr);

#endif  // ifndef BASE_DBUS_DBUS_H_