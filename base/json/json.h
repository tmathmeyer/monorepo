
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

#ifndef BASE_JSON_JSON_H_
#define BASE_JSON_JSON_H_

namespace base {
namespace json {

using Number = ssize_t;
using Float = double;

class Object;
class Array;

using JSON = std::variant<std::monostate,  // js "None"
                          Object,
                          Array,
                          std::string,
                          Number,
                          Float,
                          bool>;

bool IsNull(const JSON&);
bool IsObject(const JSON&);
bool IsArray(const JSON&);
bool IsString(const JSON&);
bool IsBool(const JSON&);
bool IsInteger(const JSON&);
bool IsFloating(const JSON&);

JSON Copy(const JSON&);
Array Copy(const Array&);
Object Copy(const Object&);

template <typename T>
std::optional<T> Unpack(JSON&& json) {
  if (std::holds_alternative<T>(json))
    return std::get<T>(std::move(json));
  return std::nullopt;
}

class Object {
 public:
  using MapType = std::map<std::string, JSON>;

  Object(MapType&&);
  const MapType& Values() const;
  size_t size() const;

  JSON operator[](std::string key) const;
  bool HasKey(std::string) const;

  Object& operator=(const Object&) = delete;
  Object(const Object&) = delete;
  Object& operator=(Object&&) = default;
  Object(Object&&) = default;

 private:
  MapType content_;
};

class Array {
 public:
  Array(std::vector<JSON>&&);
  const std::vector<JSON>& Values() const;
  std::vector<JSON> unwrap() &&;
  size_t size() const;
  Array Cdr() &&;

  JSON operator[](size_t index) const;

  Array& operator=(const Array&) = delete;
  Array(const Array&) = delete;
  Array& operator=(Array&&) = default;
  Array(Array&&) = default;

 private:
  std::vector<JSON> content_;
};

}  // namespace json
}  // namespace base

#endif  // BASE_JSON_JSON_H_
