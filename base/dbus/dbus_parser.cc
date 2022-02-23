
#include "base/dbus/dbus_parser.h"

namespace base {
namespace dbus {

base::json::JSON DecodeType(int type, DBusMessageIter *args) {
  switch(type) {
    case DBUS_TYPE_STRING: /* s */
    case DBUS_TYPE_OBJECT_PATH: /* o */{
      char *str = nullptr;
      dbus_message_iter_get_basic(args, &str);
      return std::string(str);
    }
    case DBUS_TYPE_ARRAY: /* a */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      return Iter2Array(&subtype);
    }
    case DBUS_TYPE_STRUCT: /* r */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      return Iter2Array(&subtype);
    }
    case DBUS_TYPE_DICT_ENTRY: /* e */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      auto vec = Iter2Array(&subtype);
      std::map<std::string, base::json::JSON> unwinder;
      std::optional<std::string> key = std::nullopt;
      for (const auto& v : vec.Values()) {
        if (!key.has_value()) {
          const std::string *key_v = std::get_if<std::string>(&v);
          if (!key_v)
            continue;
          key = *key_v;
        } else {
          base::json::JSON copy;
          if (const auto* ar = std::get_if<base::json::Array>(&v)) {
            if (!ar->size())
              continue;
            copy = (*ar)[0];
          } else {
            copy = base::json::Copy(v);
          }
          unwinder.insert(std::make_pair(*key, std::move(copy)));
          key = std::nullopt;
        }
      }
      //CHECK(key == std::nullopt);
      return base::json::Object(std::move(unwinder));
    }
    case DBUS_TYPE_VARIANT: /* v */{
      DBusMessageIter subtype;
      dbus_message_iter_recurse(args, &subtype);
      return UnpackVariant(&subtype);
    }
    case DBUS_TYPE_BOOLEAN: /* b */{
      bool value;
      dbus_message_iter_get_basic(args, &value);
      return value;
    }
    case DBUS_TYPE_UINT16: /* q */{
      uint16_t value;
      dbus_message_iter_get_basic(args, &value);
      return value;
    }
    case DBUS_TYPE_INT16: /* q */{
      int16_t value;
      dbus_message_iter_get_basic(args, &value);
      return value;
    }
  }

  printf("type = %c\n", type);
  //NOTREACHED();
  return {};
}

base::json::JSON UnpackVariant(DBusMessageIter *args) {
  char *sig = dbus_message_iter_get_signature(args);
  return DecodeType(sig[0], args);
}

base::json::Array Iter2Array(DBusMessageIter *args) {
  int type;
  std::vector<base::json::JSON> result;
  while ((type = dbus_message_iter_get_arg_type(args)) != DBUS_TYPE_INVALID) {
    result.push_back(DecodeType(type, args));
    dbus_message_iter_next(args);
  }
  return base::json::Array(std::move(result));
}

base::json::Object CombineKeys(base::json::Array&& values) {
  std::map<std::string, base::json::JSON> merged;
  for (const base::json::JSON& each : values.Values()) {
    const base::json::Object *maybe = std::get_if<base::json::Object>(&each);
    if (!!maybe) {
      for (const auto& kvp : maybe->Values()) {
        merged.insert({kvp.first, base::json::Copy(kvp.second)});
      }
    }
  }
  return base::json::Object(std::move(merged));
}

base::json::JSON DecodeMessageReply(DBusMessage *msg) {
  DBusMessageIter args;

  if (!dbus_message_iter_init(msg, &args))
    return {};
  
  base::json::Array result = Iter2Array(&args);

  if (result.size() == 1) {
    base::json::JSON first = std::move(std::move(result).unwrap()[0]);
    if (const base::json::Array* f = std::get_if<base::json::Array>(&first)) {
      return CombineKeys(base::json::Copy(*f));
    }
    return std::move(first);
  }

  return CombineKeys(std::move(result));
}

}  // namespace dbus
}  // namespace base