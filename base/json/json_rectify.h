
#include <optional>
#include <tuple>

#include "base/json/json.h"

#ifndef BASE_JSON_JSON_RECTIFY_H_
#define BASE_JSON_JSON_RECTIFY_H_

namespace base {
namespace json {

namespace {

template <typename T>
using Key = std::string;

}  // namespace

using namespace std;

template <typename T>
struct Unpacker {
  static std::optional<T> Run(JSON&& node) {
    return Unpack<T>(std::move(node));
  }
};

template <typename T>
struct Unpacker<std::optional<T>> {
  static std::optional<T> Run(JSON&& node) {
    return Unpack<T>(std::move(node));
  }
};

template <typename... T>
inline optional<tuple<T...>> Rectify(const Object&, Key<T>...);

template <typename F, typename... T>
inline optional<tuple<F, T...>> Rectify(const Object& o,
                                        Key<F> f,
                                        Key<T>... r) {
  optional<F> value = Unpacker<F>::Run(o[f]);
  if (!value.has_value()) {
    return nullopt;
  }
  optional<tuple<T...>> rest = Rectify<T...>(o, r...);
  if (!rest.has_value())
    return nullopt;
  return std::tuple_cat(std::tuple<F>(std::move(value).value()),
                        std::move(rest).value());
}

template <>
inline optional<tuple<>> Rectify(const Object& o) {
  std::ignore = o;
  return tuple<>();
}

template <typename D>
struct Parser {
  static std::optional<D> Parse(const JSON& j) { return Unpack<D>(Copy(j)); }
};

template <typename E>
struct Parser<std::vector<E>> {
  static std::optional<std::vector<E>> Parse(const JSON& j) {
    if (!IsArray(j))
      return std::nullopt;
    std::vector<E> result;
    auto unpacked = Unpack<Array>(Copy(j));
    for (const JSON& el : unpacked->Values()) {
      std::optional<E> maybe = Parser<E>::Parse(el);
      if (!maybe.has_value())
        return std::nullopt;
      result.push_back(std::move(maybe).value());
    }
    return result;
  }
};

template <>
struct Parser<std::tuple<>> {
  static std::optional<std::tuple<>> Parse(const JSON& j) {
    if (!IsArray(j))
      return std::nullopt;
    auto unpacked = Unpack<Array>(Copy(j));
    if (unpacked->size())
      return std::nullopt;
    return std::make_tuple<>();
  }
};

template <typename F, typename... R>
struct Parser<std::tuple<F, R...>> {
  static std::optional<std::tuple<F, R...>> Parse(const JSON& j) {
    if (!IsArray(j))
      return std::nullopt;
    auto unpacked = Unpack<Array>(Copy(j));
    if (!unpacked->size())
      return std::nullopt;
    std::optional<F> first = Parser<F>::Parse((*unpacked)[0]);
    if (!first.has_value())
      return std::nullopt;
    auto rest = Parser<std::tuple<R...>>::Parse(std::move(*unpacked).Cdr());
    if (!rest.has_value())
      return std::nullopt;
    return std::tuple_cat(std::make_tuple<F>(std::move(first).value()),
                          std::move(rest).value());
  }
};

}  // namespace json
}  // namespace base

#endif  // BASE_JSON_JSON_RECTIFY_H_
