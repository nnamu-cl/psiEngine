#pragma once
// Remove: #include <memory_resource>
#include <deque>

// Tag type for reference constructor disambiguation
struct ref_capacity_t {};
inline constexpr ref_capacity_t ref_capacity{};

template <typename T>
class CircularBuffer {
private:
    size_t m_ownedCapacity = 100;
    size_t* m_capacityPtr = nullptr;

public:
    std::deque<T> buffer;  // Changed from std::pmr::deque<T>
    size_t& capacity;

    // Default constructor
    CircularBuffer()
        : m_ownedCapacity(100)
        , m_capacityPtr(&m_ownedCapacity)
        , capacity(*m_capacityPtr)
    {}

    // Value constructor
    CircularBuffer(size_t cap)
        : m_ownedCapacity(cap)
        , m_capacityPtr(&m_ownedCapacity)
        , capacity(*m_capacityPtr)
    {}

    // Reference constructor
    CircularBuffer(ref_capacity_t, size_t& externalCapacity)
        : m_capacityPtr(&externalCapacity)
        , capacity(*m_capacityPtr)
    {}

    void Push(const T& value) {
        while (buffer.size() >= capacity) {
            buffer.pop_front();
        }
        buffer.push_back(value);
    }

    const T& operator[](size_t i) const { return buffer[i]; }
};