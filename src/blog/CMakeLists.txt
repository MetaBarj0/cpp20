Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(coroutine coroutine.cpp)

set_property(TARGET coroutine PROPERTY CXX_STANDARD 20)

target_link_libraries(coroutine PRIVATE Catch2::Catch2WithMain)
target_compile_options(coroutine PRIVATE
                       -Wall -Wextra -Wconversion -Wsign-conversion)

include(Catch)
catch_discover_tests(coroutine)

