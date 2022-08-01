Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(chapter_2_coroutines generator.cpp)

set_property(TARGET chapter_2_coroutines PROPERTY CXX_STANDARD 20)

target_link_libraries(chapter_2_coroutines PRIVATE Catch2::Catch2WithMain)
target_compile_options(chapter_2_coroutines PRIVATE
                       -Wall -Wextra -Wconversion -Wsign-conversion)

include(Catch)
catch_discover_tests(chapter_2_coroutines)

