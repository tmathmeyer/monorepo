
#include "json.h"
#include "check.h"

namespace base {

Object::Object(std::map<std::string, Value>&& map) {
  store_ = std::move(map);
}

const std::map<std::string, Value>& Object::Items() const {
  return store_;
}

void Object::SetKey(std::string key, Value&& value) {
  store_.insert({key, value});
}

Object Object::Clone() const {
  auto copy = store_;
  return Object(std::move(copy));
}

Array::Array(std::vector<Value>&& list) {
  store_ = std::move(list);
}

const std::vector<Value>& Array::Values() const {
  return store_;
}

Array Array::Clone() const {
  auto copy = store_;
  return Array(std::move(copy));
}

void Array::Append(Value&& value) {
  store_.push_back(value);
}

Value::Value(const Value& copy) {
  *this = copy.Clone();
  if (traced_)
    std::cout << "Copy to:  " << this << ": " << *this << "\n";
}

Value::Value(std::vector<Value>&& array) {
  type_ = Type::kArray;
  array_ = Array(std::move(array));
}

Value::Value(Array&& array) {
  type_ = Type::kArray;
  array_ = array;
}

Value::Value(const Array& copy) {
  type_ = Type::kArray;
  array_ = copy.Clone();
}

Value::Value(std::map<std::string, Value>&& obj) {
  type_ = Type::kObject;
  object_ = Object(std::move(obj));
}

Value::Value(Object&& object) {
  type_ = Type::kObject;
  object_ = object;
}

Value::Value(const Object& copy) {
  type_ = Type::kObject;
  object_ = copy.Clone();
}

Value::Value(std::string str, bool traced) {
  type_ = Type::kString;
  string_ = str;
  traced_ = traced;
  if (traced)
    std::cout << "Created:  " << this << ": " << str.c_str() << "\n";
}

Value::Value(const char* str) {
  type_ = Type::kString;
  string_ = str;
}

Value::Value(bool cond) {
  type_ = Type::kBoolean;
  bool_ = cond;
}

Value::Value(double floating) {
  type_ = Type::kFloating;
  double_ = floating;
}

Value::Value() {
  type_ = Type::kNull;
}

Value::~Value() {
  if (traced_) {
    std::cout << "Deleting: " << this << ": " << *this << "\n";
  }
}

Array Value::AsArray() && {
  CHECK_EQ(type_, Type::kArray);
  return std::move(*array_);
}

const Array& Value::AsArray() const & {
  CHECK_EQ(type_, Type::kArray);
  return *array_;
}

Object Value::AsObject() && {
  CHECK_EQ(type_, Type::kObject);
  return std::move(*object_);
}

const Object& Value::AsObject() const & {
  CHECK_EQ(type_, Type::kObject);
  return *object_;
}

std::string Value::AsString() && {
  CHECK_EQ(type_, Type::kString);
  return std::move(*string_);
}

std::string Value::AsString() const & {
  CHECK_EQ(type_, Type::kString);
  return *string_;
}

bool Value::AsBool() const {
  CHECK_EQ(type_, Type::kBoolean);
  return *bool_;
}

ssize_t Value::AsInteger() const {
  CHECK_EQ(type_, Type::kInteger);
  return *integer_;
}

double Value::AsDouble() const {
  CHECK_EQ(type_, Type::kFloating);
  return *double_;
}

bool Value::IsNull() const {
  return type_ == Type::kNull;
}

Value::Type Value::type() const {
  return type_;
}

Value Value::Clone() const {
  Value result;
  result.type_ = type_;
  result.string_ = string_;
  result.bool_ = bool_;
  result.object_ = object_;
  result.array_ = array_;
  result.double_ = double_;
  result.integer_ = integer_;
  result.traced_ = traced_;
  return result;
}

std::ostream& operator<<(std::ostream& stream, const Value& value) {
  value.WriteToStream(stream);
  return stream;
}

#define INDENT(indent)                \
do {                                  \
  if (indent.has_value())             \
    for (int __=0; __<*indent; __++)  \
      stream << " ";                  \
} while (0)

#define P1(indent) \
  indent.has_value() ? (*indent+1) : indent

void Value::WriteToStream(
    std::ostream& stream, std::optional<int> indent) const {
  std::optional<int> p1 = P1(indent);
  switch(type_) {
    case Value::Type::kString: {
      stream << "\"" << AsString() << "\"";
      break;
    }
    case Value::Type::kObject: {
      stream << "{";
      if (indent.has_value())
        stream << "\n";
      int first = true;
      for (const auto& kvp : AsObject().Items()) {
        if (first) {
          first = false;
        } else if (!indent.has_value()) {
          stream << ", ";
        } else {
          stream << ",\n";
        }
        INDENT(p1);
        stream << "\"" << kvp.first << "\": ";
        kvp.second.WriteToStream(stream, P1(indent));
      }
      if (indent.has_value())
        stream << "\n";
      INDENT(indent);
      stream << "}";
      break;
    }
    case Value::Type::kArray: {
      stream << "[";
      if (indent.has_value())
        stream << "\n";
      int first = true;
      for (const auto& each : AsArray().Values()) {
        if (first) {
          first = false;
        } else if (!indent.has_value()) {
          stream << ", ";
        } else {
          stream << ",\n";
        }
        INDENT(p1);
        each.WriteToStream(stream, p1);
      }
      if (indent.has_value())
        stream << "\n";
      INDENT(indent);
      stream << "]";
      break;
    }
    case Value::Type::kBoolean: {
      stream << AsBool();
      break;
    }
    case Value::Type::kInteger: {
      stream << AsInteger();
      break;
    }
    case Value::Type::kFloating: {
      stream << AsDouble();
      break;
    }
    case Value::Type::kNull: {
      stream << "null";
      break;
    }
  }
}

const Value Value::operator[](int index) const {
  const auto store = AsArray().store_;
  if (index >= store.size())
    return base::Value();
  return store[index];
}

const Value Value::operator[](std::string key) const {
  const auto store = AsObject().store_;
  const auto itr = store.find(key);
  if (itr == store.end())
    return base::Value();
  else
    return itr->second;
}


}  // namespace base