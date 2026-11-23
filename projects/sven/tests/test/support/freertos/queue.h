#pragma once
#include "FreeRTOS.h"
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <vector>

struct FakeQueue {
    size_t capacity;
    size_t itemSize;
    std::deque<std::vector<uint8_t>> items;
};
using QueueHandle_t = FakeQueue*;
namespace fake_rtos {
inline bool queueFails = false;
inline std::vector<std::unique_ptr<FakeQueue>> queues;
inline std::function<void()> afterReceive;
}
inline QueueHandle_t xQueueCreate(UBaseType_t capacity, UBaseType_t itemSize) {
    if (fake_rtos::queueFails) return nullptr;
    auto queue = std::make_unique<FakeQueue>();
    queue->capacity = capacity;
    queue->itemSize = itemSize;
    fake_rtos::queues.push_back(std::move(queue));
    return fake_rtos::queues.back().get();
}
inline BaseType_t xQueueSend(QueueHandle_t queue, const void* data, TickType_t) {
    if (queue->items.size() >= queue->capacity) return pdFALSE;
    const auto* bytes = static_cast<const uint8_t*>(data);
    queue->items.emplace_back(bytes, bytes + queue->itemSize);
    return pdTRUE;
}
inline BaseType_t xQueueReceive(QueueHandle_t queue, void* data, TickType_t) {
    if (queue->items.empty()) return pdFALSE;
    std::memcpy(data, queue->items.front().data(), queue->itemSize);
    queue->items.pop_front();
    if (fake_rtos::afterReceive) {
        auto callback = std::move(fake_rtos::afterReceive);
        fake_rtos::afterReceive = nullptr;
        callback();
    }
    return pdTRUE;
}
