#pragma once

#include <sys/mman.h>

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "../helpers/Constants.hpp"
#include "../helpers/HelperFunctions.hpp"

// open addressing hash table used for second chance replacement to keep track of currently-cached pages
struct ResidentPageSet {
    static const u64 empty = ~0ull;
    static const u64 tombstone = (~0ull) - 1;

    struct Entry {
        atomic<u64> pid;
    };

    Entry* ht;
    u64 count;
    u64 mask;
    atomic<u64> clockPos;

    ResidentPageSet(u64 maxCount) : count(next_pow2(maxCount * 1.5)), mask(count - 1), clockPos(0) {
        ht = (Entry*)allocHuge(count * sizeof(Entry));
        memset((void*)ht, 0xFF, count * sizeof(Entry));
    }

    ~ResidentPageSet() {
        munmap(ht, count * sizeof(u64));
    }

    u64 next_pow2(u64 x) {
        return 1 << (64 - __builtin_clzl(x - 1));
    }

    u64 hash(u64 k) {
        const u64 m = 0xc6a4a7935bd1e995;
        const int r = 47;
        u64 h = 0x8445d61a4e774912 ^ (8 * m);
        k *= m;
        k ^= k >> r;
        k *= m;
        h ^= k;
        h *= m;
        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        return h;
    }

    void insert(u64 pid) {
        u64 pos = hash(pid) & mask;
        while (true) {
            u64 curr = ht[pos].pid.load();
            assert(curr != pid);
            if ((curr == empty) || (curr == tombstone))
                if (ht[pos].pid.compare_exchange_strong(curr, pid))
                    return;

            pos = (pos + 1) & mask;
        }
    }

    bool remove(u64 pid) {
        u64 pos = hash(pid) & mask;
        while (true) {
            u64 curr = ht[pos].pid.load();
            if (curr == empty)
                return false;

            if (curr == pid)
                if (ht[pos].pid.compare_exchange_strong(curr, tombstone))
                    return true;

            pos = (pos + 1) & mask;
        }
    }

    template <class Fn>
    void iterateClockBatch(u64 batch, Fn fn) {
        u64 pos, newPos;
        do {
            pos = clockPos.load();
            newPos = (pos + batch) % count;
        } while (!clockPos.compare_exchange_strong(pos, newPos));

        for (u64 i = 0; i < batch; i++) {
            u64 curr = ht[pos].pid.load();
            if ((curr != tombstone) && (curr != empty))
                fn(curr);
            pos = (pos + 1) & mask;
        }
    }
};