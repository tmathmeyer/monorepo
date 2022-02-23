
#include "base/json/json.h"

namespace base {
namespace json {

bool IsNull(const JSON& json) {
  return !!std::get_if<std::monostate>(&json);
}

JSON Copy(const JSON& json) {
  if (const auto* v = std::get_if<bool>(&json))
    return *v;
  if (const auto* v = std::get_if<ssize_t>(&json))
    return *v;
  if (const auto* v = std::get_if<double>(&json))
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



Object::Object(Object::MapType&& content)
  : content_(std::move(content)) {}

const Object::MapType& Object::Values() const { return content_; }

size_t Object::size() const { return content_.size(); }

JSON Object::operator[](std::string key) const {
  decltype(content_.end()) it = content_.find(key);
  if (content_.end() != it) { 
    return Copy(it->second);
  } else {
    return {};
  }
}


Array::Array(std::vector<JSON>&& content)
  : content_(std::move(content)) {}

const std::vector<JSON>& Array::Values() const { return content_; }

size_t Array::size() const { return content_.size(); }

std::vector<JSON> Array::unwrap() && {
  return std::move(content_);
}

JSON Array::operator[](size_t index) const {
  if (index > size())
    return {};
  return Copy(content_[index]);
}

}  // namespace json
}  // namespace base