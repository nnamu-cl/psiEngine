#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "CircularBuffer.h"
#include <string>

TEST_CASE("CircularBuffer initializes with correct capacity") {
    CircularBuffer<int> buffer(10);

    REQUIRE(buffer.capacity == 10);
    REQUIRE(buffer.buffer.empty());
}

TEST_CASE("CircularBuffer uses default capacity of 100") {
    CircularBuffer<double> buffer;

    REQUIRE(buffer.capacity == 100);
    REQUIRE(buffer.buffer.empty());
}

TEST_CASE("CircularBuffer Push adds elements correctly") {
    CircularBuffer<int> buffer(5);

    SECTION("Pushing elements increases buffer size") {
        buffer.Push(1);
        REQUIRE(buffer.buffer.size() == 1);
        REQUIRE(buffer.buffer[0] == 1);

        buffer.Push(2);
        REQUIRE(buffer.buffer.size() == 2);
        REQUIRE(buffer.buffer[1] == 2);
    }

    SECTION("Pushing up to capacity does not overflow") {
        for (int i = 0; i < 5; i++) {
            buffer.Push(i);
        }
        REQUIRE(buffer.buffer.size() == 5);
        REQUIRE(buffer.buffer[0] == 0);
        REQUIRE(buffer.buffer[4] == 4);
    }
}

TEST_CASE("CircularBuffer overwrites oldest element when full") {
    CircularBuffer<int> buffer(3);

    buffer.Push(1);
    buffer.Push(2);
    buffer.Push(3);

    REQUIRE(buffer.buffer.size() == 3);
    REQUIRE(buffer.buffer[0] == 1);
    REQUIRE(buffer.buffer[1] == 2);
    REQUIRE(buffer.buffer[2] == 3);

    SECTION("Fourth element removes first") {
        buffer.Push(4);
        REQUIRE(buffer.buffer.size() == 3);
        REQUIRE(buffer.buffer[0] == 2);
        REQUIRE(buffer.buffer[1] == 3);
        REQUIRE(buffer.buffer[2] == 4);
    }

    SECTION("Multiple overwrites maintain capacity") {
        buffer.Push(4);
        buffer.Push(5);
        buffer.Push(6);

        REQUIRE(buffer.buffer.size() == 3);
        REQUIRE(buffer.buffer[0] == 4);
        REQUIRE(buffer.buffer[1] == 5);
        REQUIRE(buffer.buffer[2] == 6);
    }
}

TEST_CASE("CircularBuffer operator[] accesses elements correctly") {
    CircularBuffer<int> buffer(5);

    buffer.Push(10);
    buffer.Push(20);
    buffer.Push(30);

    REQUIRE(buffer[0] == 10);
    REQUIRE(buffer[1] == 20);
    REQUIRE(buffer[2] == 30);
}

TEST_CASE("CircularBuffer works with different types") {
    SECTION("Works with double") {
        CircularBuffer<double> buffer(3);
        buffer.Push(1.5);
        buffer.Push(2.7);
        buffer.Push(3.9);

        REQUIRE_THAT(buffer[0], Catch::Matchers::WithinRel(1.5, 0.001));
        REQUIRE_THAT(buffer[1], Catch::Matchers::WithinRel(2.7, 0.001));
        REQUIRE_THAT(buffer[2], Catch::Matchers::WithinRel(3.9, 0.001));
    }

    SECTION("Works with std::string") {
        CircularBuffer<std::string> buffer(3);
        buffer.Push("hello");
        buffer.Push("world");
        buffer.Push("test");

        REQUIRE(buffer[0] == "hello");
        REQUIRE(buffer[1] == "world");
        REQUIRE(buffer[2] == "test");

        buffer.Push("overflow");
        REQUIRE(buffer.buffer.size() == 3);
        REQUIRE(buffer[0] == "world");
        REQUIRE(buffer[1] == "test");
        REQUIRE(buffer[2] == "overflow");
    }

    SECTION("Works with custom struct") {
        struct Point {
            int x, y;
            bool operator==(const Point& other) const {
                return x == other.x && y == other.y;
            }
        };

        CircularBuffer<Point> buffer(2);
        buffer.Push({1, 2});
        buffer.Push({3, 4});

        REQUIRE(buffer[0] == Point{1, 2});
        REQUIRE(buffer[1] == Point{3, 4});

        buffer.Push({5, 6});
        REQUIRE(buffer.buffer.size() == 2);
        REQUIRE(buffer[0] == Point{3, 4});
        REQUIRE(buffer[1] == Point{5, 6});
    }
}

TEST_CASE("CircularBuffer maintains insertion order until capacity") {
    CircularBuffer<int> buffer(5);

    for (int i = 1; i <= 5; i++) {
        buffer.Push(i * 10);
    }

    REQUIRE(buffer.buffer.size() == 5);
    for (int i = 0; i < 5; i++) {
        REQUIRE(buffer[i] == (i + 1) * 10);
    }
}

TEST_CASE("CircularBuffer with capacity 1 always contains last element") {
    CircularBuffer<int> buffer(1);

    buffer.Push(1);
    REQUIRE(buffer.buffer.size() == 1);
    REQUIRE(buffer[0] == 1);

    buffer.Push(2);
    REQUIRE(buffer.buffer.size() == 1);
    REQUIRE(buffer[0] == 2);

    buffer.Push(3);
    REQUIRE(buffer.buffer.size() == 1);
    REQUIRE(buffer[0] == 3);
}

TEST_CASE("CircularBuffer stress test with many insertions") {
    CircularBuffer<int> buffer(10);

    for (int i = 0; i < 100; i++) {
        buffer.Push(i);
    }

    REQUIRE(buffer.buffer.size() == 10);
    for (int i = 0; i < 10; i++) {
        REQUIRE(buffer[i] == 90 + i);
    }
}
