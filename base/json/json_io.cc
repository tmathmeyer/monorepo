
#include "base/json/json_io.h"

namespace base {
namespace json {

namespace {

#define INDENT(_i)                    \
  do {                                \
    if (_i) {                         \
      for (int _ = 0; _ < *_i; _++) { \
        stream << "  ";               \
      }                               \
    }                                 \
  } while (0)

void WriteToStream(std::ostream& stream,
                   const JSON& json,
                   std::optional<int> indent);

void WriteToStream(std::ostream& stream,
                   const Object& value,
                   std::optional<int> indent) {
  std::optional<int> p1 = indent ? *indent + 1 : indent;
  stream << "{";
  if (indent.has_value())
    stream << "\n";
  int first = true;
  for (const auto& kvp : value.Values()) {
    if (first) {
      first = false;
    } else if (!indent.has_value()) {
      stream << ", ";
    } else {
      stream << ",\n";
    }
    INDENT(p1);
    stream << "\"" << kvp.first << "\": ";
    WriteToStream(stream, kvp.second, p1);
  }
  if (indent.has_value())
    stream << "\n";
  INDENT(indent);
  stream << "}";
}

void WriteToStream(std::ostream& stream,
                   const Array& value,
                   std::optional<int> indent) {
  std::optional<int> p1 = indent ? *indent + 1 : indent;
  stream << "[";
  if (indent.has_value())
    stream << "\n";
  int first = true;
  for (const auto& each : value.Values()) {
    if (first) {
      first = false;
    } else if (!indent.has_value()) {
      stream << ", ";
    } else {
      stream << ",\n";
    }
    INDENT(p1);
    WriteToStream(stream, each, p1);
  }
  if (indent.has_value())
    stream << "\n";
  INDENT(indent);
  stream << "]";
}

#undef INDENT

void WriteToStream(std::ostream& stream,
                   const JSON& json,
                   std::optional<int> indent) {
  std::visit(
      [&stream, indent](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
          stream << "\"" << arg << "\"";
        } else if constexpr (std::is_same_v<T, std::monostate>) {
          stream << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
          stream << (arg ? "true" : "false");
        } else if constexpr (std::is_same_v<T, ssize_t>) {
          stream << arg;
        } else if constexpr (std::is_same_v<T, double>) {
          stream << arg;
        } else if constexpr (std::is_same_v<T, Object>) {
          WriteToStream(stream, arg, indent);
        } else if constexpr (std::is_same_v<T, Array>) {
          WriteToStream(stream, arg, indent);
        }
      },
      json);
}

}  // namespace

std::ostream& operator<<(std::ostream& stream, const JSON& value) {
  std::visit(
      [&stream](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
          stream << "\"" << arg << "\"";
        } else if constexpr (std::is_same_v<T, std::monostate>) {
          stream << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
          stream << (arg ? "true" : "false");
        } else if constexpr (std::is_same_v<T, Array>) {
          WriteToStream(stream, arg, 0);
        } else if constexpr (std::is_same_v<T, Object>) {
          WriteToStream(stream, arg, 0);
        }
      },
      value);
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Object& value) {
  WriteToStream(stream, value, 0);
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const Array& value) {
  WriteToStream(stream, value, 0);
  return stream;
}

}  // namespace json
}  // namespace base