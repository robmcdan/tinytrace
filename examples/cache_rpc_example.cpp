#include <tinytrace/tinytrace.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <unordered_map>
#include <optional>

using namespace tinytrace;

// ============================================================================
// Fake cache implementation
// ============================================================================

template <typename K, typename V>
class SimpleCache {
public:
    std::optional<V> get(const K& key) {
        TRACE_SPAN("cache_get");

        // Simulate cache lookup latency
        std::this_thread::sleep_for(std::chrono::microseconds(100));

        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void put(const K& key, const V& value) {
        TRACE_SPAN("cache_put");

        // Simulate cache write latency
        std::this_thread::sleep_for(std::chrono::microseconds(150));
        data_[key] = value;
    }

private:
    std::unordered_map<K, V> data_;
};

// ============================================================================
// Fake RPC client
// ============================================================================

class RPCClient {
public:
    std::string fetch_user_data(int user_id) {
        TraceSpan rpc_span("rpc_fetch_user");

        {
            TraceSpan serialize("serialize_request");
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }

        {
            TraceSpan network("network_roundtrip");
            // Simulate variable network latency
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> latency(5, 20);
            std::this_thread::sleep_for(std::chrono::milliseconds(latency(gen)));
        }

        {
            TraceSpan deserialize("deserialize_response");
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }

        return "User data for ID " + std::to_string(user_id);
    }

    void send_notification(int user_id, const std::string& message) {
        TraceSpan rpc_span("rpc_send_notification");

        {
            TraceSpan serialize("serialize_notification");
            std::this_thread::sleep_for(std::chrono::microseconds(30));
        }

        {
            TraceSpan network("network_send");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
};

// ============================================================================
// Service layer that combines cache + RPC
// ============================================================================

class UserService {
public:
    std::string get_user(int user_id) {
        TraceSpan span("user_service_get");

        // Try cache first
        {
            auto cached = cache_.get(user_id);
            if (cached) {
                TraceSpan cache_hit("cache_hit");
                return *cached;
            }
        }

        // Cache miss - fetch from RPC
        {
            TraceSpan cache_miss("cache_miss");

            std::string user_data = rpc_.fetch_user_data(user_id);

            // Populate cache for next time
            cache_.put(user_id, user_data);

            return user_data;
        }
    }

    void notify_user(int user_id, const std::string& message) {
        TRACE_SPAN("user_service_notify");
        rpc_.send_notification(user_id, message);
    }

private:
    SimpleCache<int, std::string> cache_;
    RPCClient rpc_;
};

// ============================================================================
// Request handler (simulates API endpoint)
// ============================================================================

void handle_get_user_request(UserService& service, int user_id) {
    TraceSpan request("handle_get_user_request");

    {
        TraceSpan auth("authenticate_request");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        TraceSpan validate("validate_user_id");
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    {
        std::string user_data = service.get_user(user_id);
    }

    {
        TraceSpan respond("serialize_response");
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
}

void handle_notify_request(UserService& service, int user_id) {
    TraceSpan request("handle_notify_request");

    {
        TRACE_SPAN("authenticate_request");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    {
        service.notify_user(user_id, "Hello!");
    }
}

// ============================================================================
// Main - simulates multiple concurrent requests
// ============================================================================

int main() {
    std::cout << "Cache + RPC tracing example\n";
    std::cout << "===========================\n\n";
    std::cout << "Simulating service with:\n";
    std::cout << "  - In-memory cache\n";
    std::cout << "  - RPC backend\n";
    std::cout << "  - Multiple concurrent requests\n\n";
    std::cout << "Trace output:\n";
    std::cout << "---\n";

    UserService service;

    // Request 1: Cache miss (will hit RPC)
    std::cout << "Request 1: User 42 (cache miss)...\n";
    handle_get_user_request(service, 42);

    // Request 2: Cache hit (no RPC)
    std::cout << "Request 2: User 42 (cache hit)...\n";
    handle_get_user_request(service, 42);

    // Request 3: Different user (cache miss)
    std::cout << "Request 3: User 99 (cache miss)...\n";
    handle_get_user_request(service, 99);

    // Request 4: Notification
    std::cout << "Request 4: Notify user 42...\n";
    handle_notify_request(service, 42);

    // Concurrent requests from multiple threads
    std::cout << "\nConcurrent requests from 3 threads...\n";

    std::thread t1([&]() {
        handle_get_user_request(service, 100);
    });

    std::thread t2([&]() {
        handle_get_user_request(service, 200);
    });

    std::thread t3([&]() {
        handle_notify_request(service, 300);
    });

    t1.join();
    t2.join();
    t3.join();

    flush_traces();

    std::cout << "---\n\n";
    std::cout << "Analysis tips:\n";
    std::cout << "  1. Find cache hits: grep for 'cache_hit'\n";
    std::cout << "  2. Find cache misses: grep for 'cache_miss'\n";
    std::cout << "  3. Measure RPC latency: look at 'network_roundtrip' durations\n";
    std::cout << "  4. Compare cache vs RPC: cache_get (~100us) vs rpc_fetch_user (~5-20ms)\n";
    std::cout << "  5. Thread isolation: different thread_ids have independent span trees\n";

    return 0;
}
