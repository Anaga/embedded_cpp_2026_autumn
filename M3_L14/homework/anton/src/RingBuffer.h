/*
 * RingBuffer.h - the last N values of type T, with no heap.
 */

#pragma once

#include <stdint.h>

template <typename T, uint8_t N>
class RingBuffer {
    static_assert(N > 0U, "RingBuffer needs room for at least one item");

public:
    void push(T value) {
        m_items[m_next] = value;
        m_next = (uint8_t)((m_next + 1U) % N);
        if (m_count < N) {
            m_count++;
        }
    }

    uint8_t count(void) const {
        return m_count;
    }

    bool isFull(void) const {
        return m_count == N;
    }

    T at(uint8_t index) const {
        const uint8_t oldest = isFull() ? m_next : 0U;
        return m_items[(uint8_t)((oldest + index) % N)];
    }

    void clear(void) {
        m_next = 0U;
        m_count = 0U;
    }

private:
    T m_items[N] = {};
    uint8_t m_next = 0U;
    uint8_t m_count = 0U;
};
