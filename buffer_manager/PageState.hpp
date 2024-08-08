#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>

#include "../helpers/Constants.hpp"

struct PageState {
    atomic<u64> stateAndVersion;

    static const u64 Unlocked = 0;
    static const u64 MaxShared = 252;
    static const u64 Locked = 253;
    static const u64 Marked = 254;
    static const u64 Evicted = 255;

    PageState() {}

    void init() { stateAndVersion.store(sameVersion(0, Evicted), std::memory_order_release); }

    static inline u64 sameVersion(u64 oldStateAndVersion, u64 newState) { return ((oldStateAndVersion << 8) >> 8) | newState << 56; }
    static inline u64 nextVersion(u64 oldStateAndVersion, u64 newState) { return (((oldStateAndVersion << 8) >> 8) + 1) | newState << 56; }

    bool tryLockX(u64 oldStateAndVersion) {
        return stateAndVersion.compare_exchange_strong(oldStateAndVersion, sameVersion(oldStateAndVersion, Locked));
    }

    void unlockX() {
        assert(getState() == Locked);
        stateAndVersion.store(nextVersion(stateAndVersion.load(), Unlocked), std::memory_order_release);
    }

    void unlockXEvicted() {
        assert(getState() == Locked);
        stateAndVersion.store(nextVersion(stateAndVersion.load(), Evicted), std::memory_order_release);
    }

    void downgradeLock() {
        assert(getState() == Locked);
        stateAndVersion.store(nextVersion(stateAndVersion.load(), 1), std::memory_order_release);
    }

    bool tryLockS(u64 oldStateAndVersion) {
        u64 s = getState(oldStateAndVersion);
        if (s < MaxShared)
            return stateAndVersion.compare_exchange_strong(oldStateAndVersion, sameVersion(oldStateAndVersion, s + 1));
        if (s == Marked)
            return stateAndVersion.compare_exchange_strong(oldStateAndVersion, sameVersion(oldStateAndVersion, 1));
        return false;
    }

    void unlockS() {
        while (true) {
            u64 oldStateAndVersion = stateAndVersion.load();
            u64 state = getState(oldStateAndVersion);
            assert(state > 0 && state <= MaxShared);
            if (stateAndVersion.compare_exchange_strong(oldStateAndVersion, sameVersion(oldStateAndVersion, state - 1)))
                return;
        }
    }

    bool tryMark(u64 oldStateAndVersion) {
        assert(getState(oldStateAndVersion) == Unlocked);
        return stateAndVersion.compare_exchange_strong(oldStateAndVersion, sameVersion(oldStateAndVersion, Marked));
    }

    static u64 getState(u64 v) { return v >> 56; };
    u64 getState() { return getState(stateAndVersion.load()); }

    void operator=(PageState&) = delete;
};