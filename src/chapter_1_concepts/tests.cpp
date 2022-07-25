#include <catch2/catch_test_macros.hpp>
#include <type_traits>

template <typename T, typename... TS>
constexpr inline bool are_same_v = std::conjunction_v<std::is_same<T, TS>...>;

template <typename... Args>
requires are_same_v<Args...>
auto Add(Args &&...args) { return (args + ...); }

TEST_CASE("No compilation with a concept") {
  const int x = Add(2, 3, 4, 5);
  const int y = Add(2, 3);
  // won't compile
  // const int z = Add(2, 3.0);

  REQUIRE(x == 14);
  REQUIRE(y == 5);
  // REQUIRE(z == 5.0);
}
