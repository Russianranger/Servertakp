#pragma once
#include <array>
#include <cstdint>
#include <cmath>
#include <algorithm>

inline bool PlayerBotRestLocation(bool following,float distance_squared,unsigned follow_distance,bool at_formation) {
    const float range=std::max(3.f,std::sqrt(static_cast<float>(follow_distance)))+1.5f;
    return !following || at_formation || distance_squared<=range*range;
}
inline int PlayerBotBaseManaRegen(bool sitting,bool bard,unsigned meditate) {
    if(!sitting) return 1;
    if(bard || !meditate) return 2;
    return meditate>1 ? 4+meditate/15 : 3;
}

// A short gap between movement packets/path segments is not a rest stop.
struct PlayerBotRestGate {
    using Position = std::array<float, 3>;
    bool initialized = false;
    uint64_t last_activity = 0;
    Position owner{}, bot{};
    static bool Changed(const Position& a, const Position& b) {
        for (unsigned i = 0; i < 3; ++i)
            if (!std::isfinite(a[i]) || !std::isfinite(b[i]) || std::abs(a[i] - b[i]) > .05f) return true;
        return false;
    }
    void Reset(uint64_t now) { last_activity = now; }
    bool Update(uint64_t now, const Position& player, const Position& companion,
                bool moving, bool eligible) {
        const bool displaced = !initialized || Changed(owner, player) || Changed(bot, companion);
        if (displaced) { owner = player; bot = companion; }
        if (!initialized || displaced || moving || !eligible || now < last_activity) Reset(now);
        initialized = true;
        return eligible && !moving && !displaced && now - last_activity >= 2000;
    }
};
