#include <tinytrace/tinytrace.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace tinytrace;

void compute_fibonacci(int n) {
    TRACE_SPAN("compute_fibonacci");
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void process_request() {
    TraceSpan span("process_request");

    {
        TraceSpan auth("authenticate");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    {
        TraceSpan compute("compute");
        compute_fibonacci(10);
    }

    {
        TraceSpan respond("send_response");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

int main() {
    std::cout << "Running simple tinytrace example...\n";
    std::cout << "Trace output (JSON lines):\n";
    std::cout << "---\n";

    process_request();

    flush_traces();

    std::cout << "---\n";
    std::cout << "Done! Each line is a JSON object with:\n";
    std::cout << "  - name: span name\n";
    std::cout << "  - span_id: unique ID\n";
    std::cout << "  - parent_id: parent span ID (0 = root)\n";
    std::cout << "  - duration_us: duration in microseconds\n";
    std::cout << "  - thread_id: thread that created the span\n";

    return 0;
}
