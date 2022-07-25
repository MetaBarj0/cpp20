#include <catch2/catch_test_macros.hpp>
#include <type_traits>

template <typename T, typename... TS>
constexpr inline bool are_same_v = std::conjunction_v<std::is_same<T, TS>...>;

template <typename T, typename...> struct first_arg { using type = T; };
template <typename... TS> using first_arg_t = typename first_arg<TS...>::type;

template <typename... Args>
std::enable_if_t<are_same_v<Args...>, first_arg_t<Args...>>
Add(const Args &...args) {
  return (args + ...);
}

TEST_CASE("No compilation with SFINAE") {
  const int x = Add(2, 3, 4, 5);
  const int y = Add(2, 3);
  // won't compile
  // const int z = Add(2, 3.0);

  REQUIRE(x == 14);
  REQUIRE(y == 5);
  // REQUIRE(z == 5.0);
}
