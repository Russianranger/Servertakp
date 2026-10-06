#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace BotFormation {
constexpr float Pi = 3.14159265358979323846f;
struct Point { float x, y; };
inline Point Slot(unsigned count, unsigned index) {
    if (count == 1) return {0, -12};
    if (index < 2) return {index == 0 ? -10.f : 10.f, -12};
    if (count == 3) return {0, -24};
    if (count == 4) return {index == 2 ? -6.f : 6.f, -27};
    if (index == 2) return {0, -16};
    return {index == 3 ? -7.f : 7.f, -29};
}
inline Point Rotate(Point p, float angle) {
    return {p.x * std::cos(angle) + p.y * std::sin(angle),
            -p.x * std::sin(angle) + p.y * std::cos(angle)};
}
// Model heights give a conservative visual footprint for playable races.
inline float BodyRadius(float size) { return std::max(1.f, std::abs(size) * .35f); }
inline std::vector<Point> Layout(const std::vector<float>& sizes, float owner_size, bool gather, bool line) {
    std::vector<Point> result;
    float back = 0, previous = BodyRadius(owner_size);
    for (unsigned i=0; i<sizes.size(); ++i) {
        const float radius = BodyRadius(sizes[i]);
        if (line) {
            back += std::max(gather ? 7.f : 12.f, previous + radius + 3.f);
            result.push_back({0,-back}); previous=radius;
        } else result.push_back(Slot(sizes.size(),i));
    }
    if (!line) {
        float scale = gather ? .7f : 1.f;
        for (unsigned i=0; i<result.size(); ++i) {
            auto p=result[i];
            scale=std::max(scale,(BodyRadius(owner_size)+BodyRadius(sizes[i])+3.f)/std::hypot(p.x,p.y));
            for(unsigned j=0;j<i;++j)
                scale=std::max(scale,(BodyRadius(sizes[i])+BodyRadius(sizes[j])+3.f)/std::hypot(p.x-result[j].x,p.y-result[j].y));
        }
        for(auto& p:result) {p.x*=scale;p.y*=scale;}
    }
    return result;
}
inline void Reconcile(std::vector<uint32_t>& slots, std::vector<uint32_t> present) {
    slots.erase(std::remove_if(slots.begin(), slots.end(), [&](uint32_t id) {
        return std::find(present.begin(), present.end(), id) == present.end();
    }), slots.end());
    std::sort(present.begin(), present.end());
    for (auto id : present)
        if (std::find(slots.begin(), slots.end(), id) == slots.end() && slots.size() < 5)
            slots.push_back(id);
}
struct Frame {
    bool initialized = false;
    float x = 0, y = 0, angle = 0;
    void Update(float nx, float ny, float heading) {
        if (!initialized) { initialized = true; x = nx; y = ny; angle = heading * (2 * Pi / 256); return; }
        float dx = nx - x, dy = ny - y;
        // Accumulate small movements, but ignore turning in place and packet jitter.
        if (dx * dx + dy * dy < 4) return;
        float desired = std::atan2(dx, dy);
        float difference = std::remainder(desired - angle, 2 * Pi);
        angle += std::max(-Pi / 6, std::min(Pi / 6, difference));
        x = nx; y = ny;
    }
};
struct Clearance {
    bool active = false;
    unsigned good = 0;
    bool Update(bool clear) {
        if (!clear) { active = false; good = 0; }
        else if (++good >= 4) { active = true; good = 4; }
        return active;
    }
};
inline bool CanPosition(bool follows, bool engaged, bool casting, bool suspended, bool rooted) {
    return follows && !engaged && !casting && !suspended && !rooted;
}
inline bool Settled(float distance_squared, bool moving) {
    return distance_squared <= (moving ? 9.f : 25.f);
}
}
