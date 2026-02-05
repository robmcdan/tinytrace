add_executable(tinytrace_tests
    test_basic_span.cpp
    test_nested_spans.cpp
    test_multithreading.cpp
)

target_link_libraries(tinytrace_tests PRIVATE
    tinytrace
    Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(tinytrace_tests)
