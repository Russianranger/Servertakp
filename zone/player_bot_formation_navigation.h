#pragma once
#include "player_bot_formation.h"
#include "pathfinder_interface.h"
namespace BotFormation {
inline bool SafeRoute(IPathfinder* pathing, Map* zonemap, const glm::vec3& from, const glm::vec3& to) {
    if (!pathing || !pathing->IsUsingNavMesh() || !zonemap) return false;
    if (!zonemap->CheckLoS(from, to)) return false;
    bool partial = false, stuck = false;
    auto route = pathing->FindRoute(from, to, partial, stuck,
        PathingNotDisabled & ~(PathingZoneLine | PathingPortal | PathingLava | PathingSlime));
    if (partial || stuck || route.empty()) return false;
    float length = 0;
    auto previous = from;
    for (const auto& node : route) {
        if (node.teleport) return false;
        // Mesh points are ground positions; compare vertical differences between nodes,
        // allowing the normal model-height offset from the start/end positions.
        if (std::abs(node.pos.z - previous.z) > 20) return false;
        length += glm::length(node.pos - previous);
        previous = node.pos;
    }
    if (glm::length(glm::vec2(previous.x - to.x, previous.y - to.y)) > 3 || std::abs(previous.z - to.z) > 15) return false;
    return length <= glm::length(to - from) * 1.5f + 12;
}


}
