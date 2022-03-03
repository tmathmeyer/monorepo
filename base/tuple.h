
#include <iostream>
#include <optional>
#include <tuple>

#ifndef BASE_TUPLE_H_
#define BASE_TUPLE_H_

namespace base {

template <typename F, typename... R>
struct MetaTuple {
  template <size_t... I>
  static std::tuple<R...> __rest(std::tuple<F, R...> in,
                                 std::index_sequence<I...>) {
    return std::make_tuple(std::get<I + 1>(in)...);
  }

  static std::tuple<R...> Rest(std::tuple<F, R...> in) {
    const size_t tuple_size = std::tuple_size_v<decltype(in)>;
    auto indicies = std::make_index_sequence<tuple_size - 1>{};
    return MetaTuple<F, R...>::__rest(in, indicies);
  }
};

template <std::size_t...>
struct seq {};

template <std::size_t N, std::size_t... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct gen_seq<0, Is...> : seq<Is...> {};

template <class Ch, class Tr, class Tuple, std::size_t... Is>
void print_tuple(std::basic_ostream<Ch, Tr>& os, Tuple const& t, seq<Is...>) {
  using swallow = int[];
  (void)swallow{0,
                (void(os << (Is == 0 ? "" : ", ") << std::get<Is>(t)), 0)...};
}

}  // namespace base

template <class Ch, class Tr, class... Args>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::tuple<Args...> const& t)
    -> std::basic_ostream<Ch, Tr>& {
  os << "(";
  base::print_tuple(os, t, base::gen_seq<sizeof...(Args)>());
  return os << ")";
}

template <class Ch, class Tr, class T>
auto operator<<(std::basic_ostream<Ch, Tr>& os, std::optional<T> const& t)
    -> std::basic_ostream<Ch, Tr>& {
  if (t.has_value())
    return os << t;
  return os << "<nullopt>";
}

#endif  // BASE_TUPLE_H_
