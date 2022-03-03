
#include "base/json/json.h"

namespace base {
namespace json {

bool IsNull(const JSON& json) {
  return !!std::get_if<std::monostate>(&json);
}

bool IsObject(const JSON& json) {
  return !!std::get_if<Object>(&json);
}

bool IsArray(const JSON& json) {
  return !!std::get_if<Array>(&json);
}

bool IsString(const JSON& json) {
  return !!std::get_if<std::string>(&json);
}

bool IsBool(const JSON& json) {
  return !!std::get_if<bool>(&json);
}

bool IsInteger(const JSON& json) {
  return !!std::get_if<Number>(&json);
}

bool IsFloating(const JSON& json) {
  return !!std::get_if<Float>(&json);
}

JSON Copy(const JSON& json) {
  if (const auto* v = std::get_if<bool>(&json))
    return *v;
  if (const auto* v = std::get_if<Number>(&json))
    return *v;
  if (const auto* v = std::get_if<Float>(&json))
    return *v;
  if (const auto* v = std::get_if<std::string>(&json))
    return *v;
  if (const auto* v = std::get_if<Object>(&json))
    return Copy(*v);
  if (const auto* v = std::get_if<Array>(&json))
    return Copy(*v);
  return {};
}

Array Copy(const Array& v) {
  std::vector<JSON> copy;
  for (const auto& each : v.Values()) {
    copy.push_back(Copy(each));
  }
  return Array(std::move(copy));
}

Object Copy(const Object& v) {
  Object::MapType copy;
  for (const auto& kvp : v.Values()) {
    copy.insert({kvp.first, Copy(kvp.second)});
  }
  return Object(std::move(copy));
}

Object::Object(Object::MapType&& content) : content_(std::move(content)) {}

const Object::MapType& Object::Values() const {
  return content_;
}

size_t Object::size() const {
  return content_.size();
}

JSON Object::operator[](std::string key) const {
  decltype(content_.end()) it = content_.find(key);
  if (content_.end() != it) {
    return Copy(it->second);
  } else {
    return {};
  }
}

Array::Array(std::vector<JSON>&& content) : content_(std::move(content)) {}

const std::vector<JSON>& Array::Values() const {
  return content_;
}

size_t Array::size() const {
  return content_.size();
}

std::vector<JSON> Array::unwrap() && {
  return std::move(content_);
}

JSON Array::operator[](size_t index) const {
  if (index > size())
    return {};
  return Copy(content_[index]);
}

Array Array::Cdr() && {
  content_.erase(content_.begin());
  return std::move(*this);
}

}  // namespace json
}  // namespace base