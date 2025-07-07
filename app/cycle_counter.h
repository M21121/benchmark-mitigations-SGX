// app/cycle_counter.h
#ifndef CYCLE_COUNTER_H
#define CYCLE_COUNTER_H

#include <cstdint>

class CycleCounter {
private:
    static inline uint64_t rdtsc() {
        uint32_t lo, hi;
        __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
        return ((uint64_t)hi << 32) | lo;
    }

    static inline void cpuid() {
        __asm__ volatile ("cpuid" : : "a"(0) : "ebx", "ecx", "edx");
    }

public:
    static inline uint64_t get_cycles() {
        cpuid();
        uint64_t cycles = rdtsc();
        cpuid();
        return cycles;
    }
};

#endif // CYCLE_COUNTER_H
