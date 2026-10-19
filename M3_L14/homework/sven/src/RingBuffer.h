/*
 * RingBuffer.h - the last N values of type T, with no heap.
 * Adapted from the lesson 10 RingBuffer, with template definitions in the
 * header so it can be used with any value type and capacity.
 */

#pragma once

#include <stdint.h>

template <typename T, uint8_t N>
class RingBuffer {
    static_assert(N > 0U, "RingBuffer needs room for at least one item");

public:
    void push(T value) {
        m_items[m_next] = value;
        m_next = static_cast<uint8_t>((m_next + 1U) % N);
        if (m_count < N) {
            ++m_count;
        }
    }

    uint8_t count(void) const {
        return m_count;
    }

    // Index 0 is the oldest value, count() - 1 is the newest.
    T at(uint8_t index) const {
        const uint8_t oldest = m_count == N ? m_next : 0U;
        return m_items[static_cast<uint8_t>((oldest + index) % N)];
    }

private:
    T m_items[N] = {};
    uint8_t m_next = 0U;
    uint8_t m_count = 0U;
};
