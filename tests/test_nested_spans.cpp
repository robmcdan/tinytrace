#include <catch2/catch_test_macros.hpp>
#include <tinytrace/tinytrace.hpp>
#include <thread>
#include <chrono>

using namespace tinytrace;

TEST_CASE("Nested spans have correct parent-child relationship", "[nesting]") {
    TraceSpan parent("parent_span");
    uint64_t parent_id = parent.span_id();

    {
        TraceSpan child("child_span");
        REQUIRE(child.parent_id() == parent_id);
        REQUIRE(child.span_id() != parent_id);
    }

    // After child is destroyed, parent should still be current
    REQUIRE(parent.parent_id() == 0); // Top-level span has no parent
}

TEST_CASE("Multiple levels of nesting", "[nesting]") {
    TraceSpan level1("level_1");
    uint64_t l1_id = level1.span_id();
    REQUIRE(level1.parent_id() == 0);

    {
        TraceSpan level2("level_2");
        uint64_t l2_id = level2.span_id();
        REQUIRE(level2.parent_id() == l1_id);

        {
            TraceSpan level3("level_3");
            REQUIRE(level3.parent_id() == l2_id);

            {
                TraceSpan level4("level_4");
                REQUIRE(level4.parent_id() == level3.span_id());
            }
        }
    }
}

TEST_CASE("Sibling spans share same parent", "[nesting]") {
    TraceSpan parent("parent");
    uint64_t parent_id = parent.span_id();

    {
        TraceSpan child1("child_1");
        REQUIRE(child1.parent_id() == parent_id);
    }

    {
        TraceSpan child2("child_2");
        REQUIRE(child2.parent_id() == parent_id);
    }

    {
        TraceSpan child3("child_3");
        REQUIRE(child3.parent_id() == parent_id);
    }
}

TEST_CASE("Realistic nested operation example", "[nesting][example]") {
    // Simulates: handle_request -> db_query -> parse_results
    TraceSpan request("handle_user_request");

    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    {
        TraceSpan db("database_query");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
        TraceSpan parse("parse_results");
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

TEST_CASE("Complex nesting with multiple branches", "[nesting][example]") {
    TraceSpan request("api_request");

    {
        TraceSpan auth("authenticate");
        {
            TraceSpan cache_check("check_auth_cache");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        {
            TraceSpan db_lookup("lookup_user_db");
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    {
        TraceSpan process("process_request");
        {
            TraceSpan validate("validate_input");
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        {
            TraceSpan execute("execute_business_logic");
            {
                TraceSpan cache_read("read_cache");
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            {
                TraceSpan compute("compute_result");
                std::this_thread::sleep_for(std::chrono::milliseconds(8));
            }
        }
    }

    {
        TraceSpan respond("serialize_response");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
