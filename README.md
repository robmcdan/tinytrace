# tinytrace

A minimal, zero-dependency tracing + metrics library for C++ services

## What is it?

**tinytrace** is a header-only (or nearly) distributed tracing library designed for systems programmers who want observability without the bloat.

- **RAII-based spans** - `TraceSpan span("read_cache");` - automatic timing
- **Thread-local trace context** - nested spans work correctly across threads
- **JSON Lines output** - stdout or file, ready for your log pipeline
- **Zero dependencies** - just C++17 standard library
- **No SDK sprawl** - ~200 lines of code

## Why it exists

Most tracing libraries are bloated. We wanted something that:
- Doesn't require a PhD to integrate
- Doesn't pull in half of GitHub as dependencies
- Shows you exactly what it's doing (read the source in 5 minutes)
- Gets out of your way

Perfect for weekend projects, microservices, embedded systems, or anywhere you want lightweight observability.

## Quick Start

### 1. Clone and build

```bash
git clone https://github.com/yourusername/tinytrace.git
cd tinytrace
mkdir build && cd build
cmake ..
make
```

### 2. Run tests

```bash
ctest --output-on-failure
```

### 3. Run examples

```bash
./examples/simple_example
./examples/cache_rpc_example
```

## Usage

### Basic span

```cpp
#include <tinytrace/tinytrace.hpp>

void my_function() {
    tinytrace::TraceSpan span("my_function");
    // Your code here
    // Span automatically emits timing when it goes out of scope
}
```

### Nested spans

```cpp
void handle_request() {
    tinytrace::TraceSpan request("handle_request");

    {
        tinytrace::TraceSpan auth("authenticate");
        // Auth logic
    }

    {
        tinytrace::TraceSpan db("database_query");
        // DB logic
    }

    // Spans automatically track parent-child relationships
}
```

### Using the macro (optional)

```cpp
void process() {
    TRACE_SPAN("process");
    // Your code
}
```

### Output to file

```cpp
int main() {
    tinytrace::set_trace_output("traces.jsonl");

    // Your traced code here

    tinytrace::flush_traces();
}
```

### Output format

Each span emits a JSON line:

```json
{"name":"database_query","span_id":42,"parent_id":41,"duration_us":15234,"thread_id":"0x1234"}
```

Fields:
- `name` - span name
- `span_id` - unique span ID
- `parent_id` - parent span ID (0 = root)
- `duration_us` - duration in microseconds
- `thread_id` - thread that created the span

## Features

### RAII-based lifetime

Spans automatically start timing on construction and emit on destruction. No manual start/stop needed.

```cpp
{
    TraceSpan span("operation");
    do_work();
} // Span emitted here automatically
```

### Thread-safe

Each thread maintains its own trace context. Nested spans work correctly even with complex threading:

```cpp
std::thread t1([]() {
    TraceSpan span("thread_1");
    // Nested spans work independently per thread
});

std::thread t2([]() {
    TraceSpan span("thread_2");
    // Nested spans work independently per thread
});
```

### Exception-safe

Spans emit even if an exception is thrown:

```cpp
{
    TraceSpan span("might_throw");
    throw std::runtime_error("oops");
} // Span still emits correctly
```

## Examples

See the [examples](examples/) directory:

- **simple_example.cpp** - Basic usage
- **cache_rpc_example.cpp** - Realistic service with cache layer and RPC calls

The cache/RPC example shows:
- Cache hits vs misses
- RPC latency measurement
- Multi-threaded request handling
- Complex nested span trees

## Tests

Tests double as usage examples. See [tests](tests/):

- **test_basic_span.cpp** - Span creation, timing, RAII
- **test_nested_spans.cpp** - Parent-child relationships
- **test_multithreading.cpp** - Thread safety, worker pools

## Design philosophy

### What makes this "systems-level"?

- **RAII** - automatic resource management
- **Atomics** - lock-free span ID generation
- **thread_local** - per-thread context without locks
- **chrono + steady clocks** - monotonic timing
- **Clear performance tradeoffs** - you can see exactly what's happening

### What's NOT included (by design)

- Network exporters (use your log pipeline)
- Sampling logic (add if you need it)
- Metrics aggregation (pipe output to your metrics system)
- Complex configuration (edit the code, it's 200 lines)

This is a **building block**, not a framework.

## Performance

Overhead per span (ballpark):
- Span creation: ~100ns (atomic increment + clock read)
- Span destruction: ~1-2μs (clock read + JSON formatting + mutex lock for output)

For 99% of use cases, this is negligible. If you're tracing sub-microsecond operations, you might care.

## Stretch goals (not yet implemented)

- [ ] Sampling (trace 1/N requests)
- [ ] Compile-time enable/disable
- [ ] Perf counters (allocs, bytes)
- [ ] Chrome trace format output
- [ ] Lock-free ring buffer for output

PRs welcome!

## Integration

### CMake

```cmake
add_subdirectory(tinytrace)
target_link_libraries(your_target PRIVATE tinytrace)
```

### Header-only

Just copy `include/tinytrace/tinytrace.hpp` to your project.

## Requirements

- C++17 or later
- Standard library (chrono, thread, atomic, mutex)
- That's it!

## License

MIT - see [LICENSE](LICENSE)

## Contributing

PRs welcome! Keep it minimal and tasteful.

## Why "tinytrace"?

Because it's tiny. And it traces.

Alternative rejected names:
- microscopictrace (too long)
- nanotrace (already taken)
- smoltrace (too cute)
- trace.h (too boring)

---

**Built by systems programmers, for systems programmers.**

*No frameworks were harmed in the making of this library.*
