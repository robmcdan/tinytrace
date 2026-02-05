#include <catch2/catch_test_macros.hpp>
#include <tinytrace/tinytrace.hpp>
#include <thread>
#include <chrono>
#include <vector>

using namespace tinytrace;

TEST_CASE("Spans in different threads have independent contexts", "[threading]") {
    std::atomic<bool> thread1_started{false};
    std::atomic<bool> thread2_started{false};

    std::thread t1([&]() {
        TraceSpan span("thread_1_span");
        thread1_started = true;
        REQUIRE(span.parent_id() == 0); // No parent in this thread
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });

    std::thread t2([&]() {
        TraceSpan span("thread_2_span");
        thread2_started = true;
        REQUIRE(span.parent_id() == 0); // No parent in this thread
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });

    t1.join();
    t2.join();

    REQUIRE(thread1_started);
    REQUIRE(thread2_started);
}

TEST_CASE("Nested spans work independently per thread", "[threading][nesting]") {
    auto worker = [](int thread_num) {
        TraceSpan outer("thread_" + std::to_string(thread_num) + "_outer");
        uint64_t outer_id = outer.span_id();

        {
            TraceSpan inner("thread_" + std::to_string(thread_num) + "_inner");
            REQUIRE(inner.parent_id() == outer_id);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    };

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);

    t1.join();
    t2.join();
    t3.join();
}

TEST_CASE("Concurrent writes don't crash", "[threading][stress]") {
    constexpr int num_threads = 10;
    constexpr int spans_per_thread = 100;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, spans_per_thread]() {
            for (int j = 0; j < spans_per_thread; ++j) {
                TraceSpan span("thread_" + std::to_string(i) + "_span_" + std::to_string(j));
                // Minimal work
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    REQUIRE(true); // If we got here without crashing, success!
}

TEST_CASE("Simulated worker pool pattern", "[threading][example]") {
    auto process_job = [](int job_id) {
        TraceSpan job_span("process_job_" + std::to_string(job_id));

        {
            TraceSpan fetch("fetch_data");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

        {
            TraceSpan compute("compute");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        {
            TraceSpan store("store_result");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    std::vector<std::thread> workers;
    for (int i = 0; i < 5; ++i) {
        workers.emplace_back(process_job, i);
    }

    for (auto& w : workers) {
        w.join();
    }
}

TEST_CASE("Thread pool with nested work stealing pattern", "[threading][example]") {
    auto steal_work = [](int from_thread, int to_thread) {
        TraceSpan steal("steal_from_" + std::to_string(from_thread) +
                        "_to_" + std::to_string(to_thread));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    };

    auto worker = [&steal_work](int thread_id) {
        TraceSpan worker_span("worker_" + std::to_string(thread_id));

        for (int task = 0; task < 3; ++task) {
            TraceSpan task_span("task_" + std::to_string(task));
            std::this_thread::sleep_for(std::chrono::milliseconds(2));

            // Simulate work stealing
            if (task == 1) {
                steal_work(thread_id, (thread_id + 1) % 4);
            }
        }
    };

    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i) {
        pool.emplace_back(worker, i);
    }

    for (auto& t : pool) {
        t.join();
    }
}
