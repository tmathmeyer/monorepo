#ifndef BASE_SERIALIZE_BASE_H_
#define BASE_SERIALIZE_BASE_H_

#include <vector>

#include "json.h"

namespace base {

namespace internal {

template <typename T>
struct Serializer {
  static inline base::Value Convert(const T& value) {
    return Value(value);
  }
};

}  // namespace internal

template <typename T>
base::Value Serialize(const T& t) {
  return internal::Serializer<T>::Convert(t);
}

}  // namespace base

#endif  // BASE_SERIALIZE_BASE_H_