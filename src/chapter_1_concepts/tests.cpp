#include <array>
#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <type_traits>

template <typename T, typename... TS>
constexpr inline bool are_same_v = std::conjunction_v<std::is_same<T, TS>...>;

template <typename T, typename...> struct first_arg {
  using type = T;
  constexpr static auto value = 42;
};

template <typename... TS> using first_arg_t = typename first_arg<TS...>::type;

template <typename... Args>
std::enable_if_t<are_same_v<Args...>, first_arg_t<Args...>>
add_sfinae(Args &&...args) {
  return (... + args);
}

template <typename... Args>
requires are_same_v<Args...>
auto add_concepts(Args &&...args) { return (... + args); }

TEST_CASE("No compilation with SFINAE") {
  const int x = add_sfinae(2, 3, 4, 5);
  const int y = add_sfinae(2, 3);
  // won't compile
  // const int z = add_sfinae(2, 3.0);

  REQUIRE(x == 14);
  REQUIRE(y == 5);
  // REQUIRE(z == 5.0);
}

TEST_CASE("No compilation with concepts") {
  const int x = add_concepts(2, 3, 4, 5);
  const int y = add_concepts(2, 3);
  // won't compile
  // const int z = add_concepts(2, 3.0);

  REQUIRE(x == 14);
  REQUIRE(y == 5);
  // REQUIRE(z == 5.0);
}

struct simple_requirement_data {
  template <typename... Args>
  // specifying ad-hoc constraints
  // parameters of the requires statement are unevaluated operands
  requires requires(Args... args) {
    // simple requirement expression regarding values
    (... + args);
    // following are nested requirement, omitting `requires` would make them
    // simple requirements. In case of `B`, omitting `requires` in front would
    // always yield `true` as it's always possible to use sizeof... on a
    // parameter pack
    requires are_same_v<Args...>;
    requires sizeof...(Args) > 1; // B
    // Following is a compound type requirement.
    // Compound type requirement are used to check the return type and the
    // noexceptness of an expression.
    // curly braces form a scope or compound statement
    // return type constraint must be specified by a concept, not a type trait.
    // The compiler inject the first template parameter type in std::same_as
    // concept
    { (... + args) }
    noexcept->std::same_as<first_arg_t<Args...>>;
    // following is a type requirement that is not useful but for demonstration
    // purpose. Both type and value requirement are shown.
    typename first_arg<Args...>::type;
    first_arg<Args...>::value;
  }
  auto add(Args &&...args) { return (... + args); }
};

TEST_CASE("requires expressions") {
  simple_requirement_data data;

  const int x = data.add(2, 3, 4, 5);
  const int y = data.add(2, 3);
  // won't compile
  // const int z = data.add(2, 3.0);

  REQUIRE(x == 14);
  REQUIRE(y == 5);
  // REQUIRE(z == 5.0);
}

template <typename Arg>
concept Addable = requires(Arg arg) {
  { arg + arg }
  noexcept->std::same_as<Arg>;
};

template <typename... Args>
concept Adder = requires(Args... args) {
  requires are_same_v<Args...>;
  requires sizeof...(Args) > 1;
};

struct concept_data {
  auto add(Addable auto &&...args) requires Adder<decltype(args)...> {
    return (... + args);
  }
};

TEST_CASE("creating a concept") {
  concept_data data;

  const int x = data.add(2, 3, 4, 5);
  const int y = data.add(2, 3);
  // won't compile
  // const int z = data.add(2, 3.0);

  REQUIRE(x == 14);
  REQUIRE(y == 5);
  // REQUIRE(z == 5.0);
}

struct constexpr_function_in_concept_data {
  // consteval function with default argument for template type parameter. Easy
  // usage in the constraint definition.
  // Moreover, keep the template type instance in an unevaluated context.
  // using constrained auto return type syntax
  static consteval std::integral auto get_size(const auto &container = {}) {
    return container.size();
  }
};

template <typename T, std::size_t N>
// shorthand notation for concept, no require block.
// makes use of the constexpr function, leveraging default parameter to stay in
// an unevaluated context.
concept n_sized = (constexpr_function_in_concept_data::get_size<T>() == N);

// alternative notation emphasizing that constraints parameter stay in an
// unevaluated context; the requires clause does not evaluate T (it has no
// parameter)
template <typename T, std::size_t N>
concept n_sized_alt = requires {
  requires constexpr_function_in_concept_data::get_size<T>()
  == N;
};

// specifying the concept before an auto template parameter.
// The const qualifier must be set before the auto keyword.
void send_ping(const n_sized<1> auto &) {}
void send_ping_alt(const n_sized<1> auto &) {}

TEST_CASE("constexpr function in concept") {
  std::array a{42};
  send_ping(a);
  send_ping_alt(a);

  static_assert(n_sized<decltype(a), 1>);
  static_assert(n_sized_alt<decltype(a), 1>);
}

template <typename T>
concept validable = requires(const T t) {
  t.validate();
};

struct foo {
  void validate() {}
};

struct bar {};

template <typename T> struct validator {
  void proceed(const T &t) {
    if constexpr (validable<T>)
      t.validate();
  }
};

TEST_CASE("using constraint in function") {
  foo my_foo;
  bar my_bar;

  validator<bar> bar_validator;
  bar_validator.proceed(my_bar);

  validator<foo> foo_validator;
  foo_validator.proceed(my_foo);
}
