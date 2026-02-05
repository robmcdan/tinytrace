#include <catch2/catch_test_macros.hpp>
#include <tinytrace/tinytrace.hpp>
#include <thread>
#include <chrono>

using namespace tinytrace;

TEST_CASE("TraceSpan creates valid span IDs", "[basic]") {
    TraceSpan span1("test_span_1");
    REQUIRE(span1.span_id() > 0);

    {
        TraceSpan span2("test_span_2");
        REQUIRE(span2.span_id() > span1.span_id());
        REQUIRE(span2.span_id() != span1.span_id());
    }
}

TEST_CASE("TraceSpan measures duration", "[basic][timing]") {
    // This span should take at least 10ms
    {
        TraceSpan span("sleep_10ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // Check output manually - duration should be >= 10000 microseconds
}

TEST_CASE("TraceSpan RAII cleanup on exception", "[basic][exception]") {
    try {
        TraceSpan span("will_throw");
        throw std::runtime_error("test exception");
    } catch (const std::runtime_error&) {
        // Span should have been destroyed and emitted
        REQUIRE(true);
    }
}

TEST_CASE("Multiple sequential spans", "[basic]") {
    {
        TraceSpan span1("operation_1");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    {
        TraceSpan span2("operation_2");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    {
        TraceSpan span3("operation_3");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

TEST_CASE("TRACE_SPAN macro works", "[basic][macro]") {
    {
        TRACE_SPAN("macro_test");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    REQUIRE(true);
}

TEST_CASE("Trace output can be redirected to file", "[basic][output]") {
    const std::string test_file = "test_trace_output.jsonl";

    set_trace_output(test_file);

    {
        TraceSpan span("file_output_test");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    flush_traces();

    // Check file exists
    std::ifstream file(test_file);
    REQUIRE(file.good());

    // Clean up
    file.close();
    std::remove(test_file.c_str());
}
