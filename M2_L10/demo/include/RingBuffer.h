/*
 * RingBuffer.h - the last N values of type T, with no heap.
 *
 * The size N is fixed when the program is compiled. The memory for all N
 * items is part of the object itself, so sizeof(RingBuffer<T, N>) is known
 * in advance and nothing is ever allocated while the program runs.
 *
 * When the buffer is full, push() overwrites the oldest value.
 *
 * A template lives entirely in its header: the compiler needs to see the
 * whole code to write a version for each T and N you use.
 */

#pragma once

#include <stdint.h>

template <typename T, uint8_t N>
class RingBuffer {
    static_assert(N > 0U, "RingBuffer needs room for at least one item");

public:
    void push(T value) {
        m_items[m_next] = value;
        m_next = (uint8_t)((m_next + 1U) % N);   // after the last place: 0
        if (m_count < N) {
            m_count++;
        }
    }

    /* How many values are held right now: 0 to N. */
    uint8_t count(void) const {
        return m_count;
    }

    bool isFull(void) const {
        return m_count == N;
    }

    /* index 0 is the oldest value still held, count() - 1 the newest. */
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
    uint8_t m_next = 0U;    // where the next value will go
    uint8_t m_count = 0U;
};
