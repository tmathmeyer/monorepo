#ifndef BASE_SERIALIZE_H_
#define BASE_SERIALIZE_H_

#include "json.h"
#include "location.h"
#include "serialize_base.h"
#include "status.h"

namespace base {
namespace internal {

#define FIELD_SERIALIZE(NAME, CONSTEXPR) \
  result.SetKey(NAME, Serialize(CONSTEXPR))

template <typename T>
struct Serializer<const T> {
  static base::Value Convert(const T& value) {
    return Serializer<T>::Convert(value);
  }
};

template <typename VecType>
struct Serializer<std::vector<VecType>> {
  static Value Convert(const std::vector<VecType>& vec) {
    Array result;
    for (const VecType& value : vec)
      result.Append(Serializer<VecType>::Convert(value));
    return Value(std::move(result));
  }
};

template <int len>
struct Serializer<char[len]> {
  static inline Value Convert(const char* code) {
    return Value(code);
  }
};

template <>
struct Serializer<char*> {
  static inline Value Convert(const char* code) {
    return Value(code);
  }
};

template <>
struct Serializer<Location> {
  static Value Convert(const Location& value) {
    Object result;
    FIELD_SERIALIZE("file", value.filename);
    FIELD_SERIALIZE("line", value.line_number);
    return Value(std::move(result));
  }
};

template <>
struct Serializer<std::string> {
  static Value Convert(const std::string& value) {
    return Value(value);
  }
};

template<>
struct Serializer<StatusData> {
  static Value Convert(const StatusData& value) {
    Object result;
    FIELD_SERIALIZE("group", value.group);
    FIELD_SERIALIZE("code", value.code);
    FIELD_SERIALIZE("message", value.message);
    FIELD_SERIALIZE("frames", value.frames);
    FIELD_SERIALIZE("causes", value.causes);
    FIELD_SERIALIZE("data", value.data.Clone());
    return Value(std::move(result));
  }
};

template<typename T>
struct Serializer<TypedStatus<T>> {
  static Value Convert(const TypedStatus<T>& value) {
    if (value.is_ok())
      return Value("Ok");
    return Serializer<StatusData>::Convert(*(value.data_));
  }
};

}  // namespace internal
}  // namespace base

#endif  // BASE_SERIALIZE_H_