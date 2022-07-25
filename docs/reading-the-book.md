# Programming with C++ 20

Written by Andreas Fertig.

Gonna study Concepts, Coroutines, Ranges and more.

## Practical aspects

The book examples run with clang 13, clang 14 should be ok.
The code is experienced through test cases using catch2

## Chapter 1: Concepts

They are predicates for strongly typed generic code.

They facilitate te writing of template meta programming code.
They provide better and more explicit error messages for generic template meta programming code misusage.

### Playing with examples

Got to src/chapter_1_concepts/tests.cpp to get a grasp on the matter.

### specific remarks

Table 1.1

```cpp
template<C1 T>
requires C2<T>
C3 auto Fun(C4 auto param) requires C5<T>)
```

| ref | type constraint          | when to use                                                                                           |
|----------------------------------------------------------------------------------------------------------------------------------------|
| C1  | type constraint          | Use this when you already know that a template type parameter has a certain constraint                |
| C2  | requires clause          | Use this when you need to add constraints for multiple template type or non-type template parameters. |
| C5  | trailing requires clause | Use this for a method in a class template to constrain it based on the class template parameters.     |

### 1.5 Body of a require expression

Four different forms:

- Simple requirement   (SR)
- Nested requirement   (NR)
- Compound requirement (CR)
- type requirement     (TR)

Go to the code to see how those requirements are hande in an ad-hoc constraint expression.

### 1.7 defining a concept

```cpp
// concept head
template<typename T, typename U>
concept MyConcept /*concept name*/ =
  std::same_as<T, U> &&  // requirement
  (std::is_class_v<T>    // requirement
  || std::is_enum_v<T>); // requirement
```

### 1.8 testing requirements

They are a evaluated at compile time. The compiler is sufficient to test them, no need of a test framework.
A bunch of well thought static_assert should do the job.
Actually, the Addable concept studied in the book can be implemented using some sort of TDD at compile time.

TODO: write an article on the matter
