#include <catch2/catch_test_macros.hpp>
#include <coroutine>

namespace {
auto coroutine() {
  co_yield "hello ";
  co_yield "world";
  co_return "!";
}

TEST_CASE("test") { REQUIRE(true); }
} // namespace
