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

### 1.10 using constexpr function inside a concept

const(expr|eval) functions are usable in a concept as soon as one doesn't use
values used in the requirement definition

### 1.12.1 using concept in function body

Thanks to `if constexpr` construction, one can verify constraints at compile time
and generate only the necessary code.
We can even conditionaly generate constructors and destructors.
Concept subsumption is resolved using concept only, not ad-hoc constraints
using type traits for instance. Using ad-hoc constraints prevent the compiler
to correctly subsume constraints and may lead to ambiguity errors. One can see
subsumable constraints defined with concept as Venn diagrams:

```
+-C2-----+
|        |
| +-C1-+ |
| |    | |
| +----+ |
|        |
+--------+
```
One can imagine the free space in the boxes as constraints (either from compile
time values or other concepts).
Designing concept as above is ok for the compiler to subsume them correctly.

On the other hand, designing concept like in the following diagram does not
allow the compiler to resolve symbols without ambiguity:

```
+-C2---+
|      |
|    +---C1-+
|    |      |
|    +------+
|      |
+------+
```

Above we can see C1 and C2 concepts have only a part of their constraint in
common, making subsumption impossible as they are partially disjoint.

Similarly, using the logical `not` to apply constraints with concept is not recommended:

```
+-C2-+ +-not C2 aka. all the universe but C1-+
|    | |                                     |
+----+ +-------------------------------------+
```

Morever, specfying `not` several times at different locations makes the
compiler deduces the source location of the used concept are different and
deduces that concepts are different, preventing subsumption. Weird huh? A way
to see things more simply is to consider using `not` with a concept the same
thing as specifying ad-hoc requirements.
