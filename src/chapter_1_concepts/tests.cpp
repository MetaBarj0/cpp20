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

namespace {
struct copyable {
  copyable() = default;

  copyable(const copyable &) = default;
  copyable &operator=(const copyable &) = default;

  ~copyable() {}
};

struct non_copyable {
  non_copyable() = default;

  non_copyable(const non_copyable &) = delete;
  non_copyable &operator=(const non_copyable &) = delete;

  ~non_copyable() = default;
};

struct com_like {
  ~com_like() {}
  void release() {}
};

template <typename T>
concept has_release = requires(T t) {
  t.release();
};

template <typename T>
concept not_trivially_destructible = not std::is_trivially_destructible_v<T>;

template <typename T> struct wrapper {
  wrapper() requires std::is_default_constructible_v<T>
  = default;

  wrapper(const wrapper &) requires std::is_copy_assignable_v<T>
  = default;

  ~wrapper() = default;

  ~wrapper() requires not_trivially_destructible<T> and has_release<T> {
    t.release();
    t.~T();
  }

  ~wrapper() requires not_trivially_destructible<T> { t.~T(); }

private:
  T t;
};

TEST_CASE("conditional ctor/dtor") {
  wrapper<copyable> copyable_wrapper;
  wrapper<non_copyable> non_copyable_wrapper;
  (void)non_copyable_wrapper;
  wrapper<com_like> com_wrapper;

  decltype(auto) another_copy = copyable_wrapper;
  (void)another_copy;

  // does not compile, because of no matching ctor
  // decltype(auto) another_copy = non_copyable_wrapper;
}

} // namespace
namespace {
template <typename T, typename U>
concept is_same_ordered = std::is_same_v<T, U>;

// to remove ambiguity, regarding ordering
template <typename T, typename U>
concept is_same = is_same_ordered<T, U> and is_same_ordered<U, T> and
    is_same_ordered<U, U> and is_same_ordered<T, T>;

template <typename...>
concept always_true = true;

template <typename A, typename B>
requires is_same<A, B>
auto subsuming_add(A &&a, B &&b) { return a + b; }

template <typename A, typename B>
requires is_same<A, A>
auto subsuming_add(A &&a, A &&b) { return a + b; }

template <typename A, typename B>
requires is_same<B, A> and always_true<B>
auto subsuming_add(A &&a, B &&b) { return a + b; }

template <typename A, typename B>
requires is_same<B, B> and always_true<B>
auto subsuming_add(B &&a, B &&b) { return a + b; }

template <typename A, typename B>
requires is_same<A, A> and always_true<A>
auto subsuming_add(A &&a, A &&b) { return a + b; }

TEST_CASE("subsumption exploring") {
  constexpr int a = 1, b = 2;

  // the most constrained one will be selected
  subsuming_add(a, b);
}
} // namespace
