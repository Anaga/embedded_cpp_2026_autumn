/*
 * RingBuffer.cpp - fixed-capacity history, oldest value first.
 */

#include "RingBuffer.h"

template <typename T, uint8_t N>
void RingBuffer<T, N>::push(T value) {
    m_items[m_next] = value;
    m_next = (uint8_t)((m_next + 1U) % N);
    if (m_count < N) {
        m_count++;
    }
}

template <typename T, uint8_t N>
uint8_t RingBuffer<T, N>::count() const {
    return m_count;
}

template <typename T, uint8_t N>
bool RingBuffer<T, N>::isFull() const {
    return m_count == N;
}

template <typename T, uint8_t N>
T RingBuffer<T, N>::at(uint8_t index) const {
    const uint8_t oldest = isFull() ? m_next : 0U;
    return m_items[(uint8_t)((oldest + index) % N)];
}

template <typename T, uint8_t N>
void RingBuffer<T, N>::clear() {
    m_next = 0U;
    m_count = 0U;
}

template class RingBuffer<uint16_t, 10>;
