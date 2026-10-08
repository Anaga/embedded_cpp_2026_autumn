/*
 * RingBuffer.h - the last N values of type T, with no heap.
 *
 * The size N is fixed when the program is compiled. The memory for all N
 * items is part of the object itself, so sizeof(RingBuffer<T, N>) is known
 * in advance and nothing is ever allocated while the program runs.
 *
 * When the buffer is full, push() overwrites the oldest value.
 *
 * Definitions live in RingBuffer.cpp, which explicitly instantiates
 * RingBuffer<uint16_t, 10>. Add an explicit instantiation there before
 * using another type or capacity.
 */

#pragma once

#include <stdint.h>

template <typename T, uint8_t N>
class RingBuffer {
    static_assert(N > 0U, "RingBuffer needs room for at least one item");

public:
    void push(T value);

    /* How many values are held right now: 0 to N. */
    uint8_t count() const;

    bool isFull() const;

    /* index 0 is the oldest value still held, count() - 1 the newest. */
    T at(uint8_t index) const;

    void clear();

private:
    T m_items[N] = {};
    uint8_t m_next = 0U;    // where the next value will go
    uint8_t m_count = 0U;
};

extern template class RingBuffer<uint16_t, 10>;
