
#ifndef BASE_DBUS_DBUS_PARSER_H_
#define BASE_DBUS_DBUS_PARSER_H_

#include <cstdio>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include <dbus/dbus.h>

#include "base/json/json.h"

namespace base {
namespace dbus {

// Convert a DBus iterator into a vector of Values.
base::json::JSON Iter2JSON(DBusMessageIter* args);

// Convert a variant into a Value.
base::json::JSON UnpackVariant(DBusMessageIter* args);

// Convert an individual, labeled type.
base::json::JSON DecodeType(int type, DBusMessageIter* args);

// Maybe pull a value from a message. nullopt if void.
base::json::JSON DecodeMessageReply(DBusMessage* msg);

}  // namespace dbus
}  // namespace base

#endif  // ifndef BASE_DBUS_DBUS_PARSER_H_