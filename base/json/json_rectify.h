
#include <optional>
#include <tuple>


#include "base/json/json.h"

#ifndef BASE_JSON_JSON_RECTIFY_H_
#define BASE_JSON_JSON_RECTIFY_H_

namespace base {
namespace json {

namespace {

template<typename T>
using Key = std::string;

}  // namespace

using namespace std;

template<typename... T>
inline optional<tuple<T...>> Rectify(const Object&, Key<T>...);

template<typename F, typename... T>
inline optional<tuple<F, T...>> Rectify(const Object& o, Key<F> f, Key<T>... r) {
  optional<F> value = base::json::Unpack<F>(o[f]);
  if (!value.has_value())
    return nullopt;
  optional<tuple<T...>> rest = Rectify<T...>(o, r...);
  if (!rest.has_value())
    return nullopt;
  return std::tuple_cat(
    std::tuple<F>(std::move(value).value()), std::move(rest).value());
}

template<>
inline optional<tuple<>> Rectify(const Object& o) {
  return tuple<>();
}

}
}

#endif  // BASE_JSON_JSON_RECTIFY_H_
