#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <coroutine>
#include <exception>
#include <iostream>
#include <optional>

class Future;

struct Promise {
  using value_type = const char *;
  const char *value{};

  Promise() = default;
  std::suspend_always initial_suspend() { return {}; }
  std::suspend_always final_suspend() noexcept { return {}; }
  void unhandled_exception() {
    std::rethrow_exception(std::current_exception());
  }

  std::suspend_always yield_value(const char *value) {
    this->value = std::move(value);
    return {};
  }

  void return_void() { this->value = nullptr; }

  Future get_return_object();
};

class Future {
public:
  using promise_type = Promise;

  explicit Future(std::coroutine_handle<Promise> handle) : handle(handle) {}

  ~Future() {
    if (handle) {
      handle.destroy();
    }
  }

  Promise::value_type next() {
    if (handle) {
      handle.resume();
      return handle.promise().value;
    } else {
      return {};
    }
  }

private:
  std::coroutine_handle<Promise> handle{};
};

Future Promise::get_return_object() {
  return Future{std::coroutine_handle<Promise>::from_promise(*this)};
}

Future Generator() {
  co_yield "Hello ";
  co_yield "world";
  co_yield "!";
}

TEST_CASE("coroutine generator from blog") {
  auto generator = Generator();

  using Catch::Matchers::Equals;

  REQUIRE_THAT(generator.next(), Equals("Hello "));
  REQUIRE_THAT(generator.next(), Equals("world"));
  REQUIRE_THAT(generator.next(), Equals("!"));
}

