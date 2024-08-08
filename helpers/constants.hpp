#pragma once

#include <cstdint>
using namespace std;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u64 PID; // page id type
// Defined the type for SSTable type
typedef u64 TableId;

static const u64 pageSize = 4096;

struct alignas(4096) Page {
   bool dirty;
};

struct SSTable {
   bool dirty;
};

static const int16_t maxWorkerThreads = 128;

#define die(msg) do { perror(msg); exit(EXIT_FAILURE); } while(0)
