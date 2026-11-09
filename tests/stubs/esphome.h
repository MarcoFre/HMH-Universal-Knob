#pragma once
#include <cstdint>
extern uint32_t fake_millis;
inline uint32_t millis() {return fake_millis;}
