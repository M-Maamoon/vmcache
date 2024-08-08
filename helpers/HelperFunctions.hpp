#pragma once
#include <sys/mman.h>  // Add this line to include the header file for madvise
#include <xmmintrin.h>

#include "Constants.hpp"

void* allocHuge(size_t size) {
    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    madvise(p, size, MADV_HUGEPAGE);
    return p;
}

void yield(u64 counter) {
    _mm_pause();
}

uint64_t rdtsc() {
    uint32_t hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return static_cast<uint64_t>(lo) | (static_cast<uint64_t>(hi) << 32);
}

static unsigned min(unsigned a, unsigned b) {
    return a < b ? a : b;
}

template <class T>
static T loadUnaligned(void* p) {
    T x;
    memcpy(&x, p, sizeof(T));
    return x;
}
