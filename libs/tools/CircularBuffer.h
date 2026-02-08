#pragma once
#include <deque>
#include <memory_resource>

// Tag type for reference constructor disambiguation
struct ref_capacity_t {};
inline constexpr ref_capacity_t ref_capacity{};

template <typename T>
class CircularBuffer {
private:
    size_t m_ownedCapacity = 100;  // owned capacity for value-based construction
    size_t* m_capacityPtr = nullptr;  // pointer to capacity (owned or external)

public:
    std::pmr::deque<T> buffer;

    // Reference to capacity (either owned or external)
    size_t& capacity;

    // Default constructor - uses internal capacity of 100
    CircularBuffer()
        : m_ownedCapacity(100)
        , m_capacityPtr(&m_ownedCapacity)
        , capacity(*m_capacityPtr)
    {}

    // Value constructor - takes capacity by value and stores it
    CircularBuffer(size_t cap)
        : m_ownedCapacity(cap)
        , m_capacityPtr(&m_ownedCapacity)
        , capacity(*m_capacityPtr)
    {}

    // Reference constructor - references external capacity (for LineGraphNode use case)
    // Uses tag dispatch to disambiguate from value constructor
    CircularBuffer(ref_capacity_t, size_t& externalCapacity)
        : m_capacityPtr(&externalCapacity)
        , capacity(*m_capacityPtr)
    {}

    void Push(const T& value) {
        while (buffer.size() >= capacity) {
            buffer.pop_front(); // remove the oldest member
        }
        buffer.push_back(value);
    }

    const T& operator[](size_t i) const { return buffer[i]; }
};


