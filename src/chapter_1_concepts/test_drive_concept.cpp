#include <concepts>
#include <type_traits>

template <typename Arg, typename... Args>
static constexpr auto are_same_v =
    std::conjunction_v<std::is_same<Arg, Args>...>;

template <typename T, typename... Ts> struct first_type_of { using type = T; };

template <typename... Ts>
using first_type_of_t = typename first_type_of<Ts...>::type;

template <typename... Args>
concept Adder = requires(Args... args) {
  requires sizeof...(Args) > 1;
  requires are_same_v<Args...>;
  { (... + args) }
  noexcept->std::same_as<first_type_of_t<Args...>>;
};

consteval void adder_should_be_applicable_on_a_function() {
  (void)[]<typename... Args>
  requires Adder<Args...>(Args && ...) {};
}

consteval void adder_function_should_at_least_have_two_arguments() {
  static_assert(not Adder<>);
  static_assert(not Adder<int>);
}

consteval void adder_function_parameters_should_all_be_oof_same_type() {
  static_assert(not Adder<int, float>);
}

consteval void adder_function_parameters_should_be_addable_and_noexcept() {
  struct foo {};

  struct addable_throwing {
    constexpr auto operator+(addable_throwing) const {
      return addable_throwing{};
    }
  };

  struct addable_noexcept {
    constexpr auto operator+(addable_noexcept) const noexcept {
      return addable_noexcept{};
    }
  };

  static_assert(not Adder<foo, foo>);
  static_assert(not Adder<addable_throwing, addable_throwing>);
  static_assert(Adder<addable_noexcept, addable_noexcept>);
}

consteval void adder_function_return_type_should_be_the_same_as_arguments() {
  struct addable_conversion {
    constexpr int operator+(addable_conversion) const noexcept {
      return static_cast<int>(addable_conversion{});
    }

    constexpr explicit operator int() const noexcept { return 42; }
  };

  struct addable {
    constexpr auto operator+(addable) const noexcept { return addable{}; }
  };

  static_assert(not Adder<addable_conversion, addable_conversion>);
  static_assert(Adder<addable, addable>);
}
