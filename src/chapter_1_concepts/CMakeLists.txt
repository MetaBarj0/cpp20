Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(chapter_1_concepts "tests.cpp")

set_property(TARGET chapter_1_concepts PROPERTY CXX_STANDARD 20)

target_link_libraries(chapter_1_concepts PRIVATE Catch2::Catch2WithMain)
target_compile_options(chapter_1_concepts PRIVATE
                       -Wall -Wextra -Wconversion -Wsign-conversion)

include(Catch)
catch_discover_tests(chapter_1_concepts)

