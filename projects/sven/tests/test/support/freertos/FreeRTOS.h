#pragma once
#include <cstddef>
#include <cstdint>

using BaseType_t = int;
using UBaseType_t = unsigned;
using TickType_t = uint32_t;
using portMUX_TYPE = int;
constexpr int portMUX_INITIALIZER_UNLOCKED = 0;
constexpr BaseType_t pdTRUE = 1;
constexpr BaseType_t pdFALSE = 0;
namespace fake_rtos {
inline int lockDepth = 0;
}
inline void portENTER_CRITICAL(portMUX_TYPE*) { ++fake_rtos::lockDepth; }
inline void portEXIT_CRITICAL(portMUX_TYPE*) { --fake_rtos::lockDepth; }
inline void portENTER_CRITICAL_ISR(portMUX_TYPE* mux) { portENTER_CRITICAL(mux); }
inline void portEXIT_CRITICAL_ISR(portMUX_TYPE* mux) { portEXIT_CRITICAL(mux); }
