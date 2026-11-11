#pragma once

#include <cstdio>
#include <cstring>

#include "particles.hpp"

inline Particle sample_particle(std::uint32_t i) {
    Particle p{};
    p.x = static_cast<float>(i % 17) - 8;
    p.y = static_cast<float>(i % 5) * 1.5f;
    p.z = static_cast<float>(i * 37 % 100) / 4;
    p.vx = static_cast<float>(i % 3) - 1;
    p.vy = static_cast<float>(i % 9) / 2;
    p.vz = static_cast<float>(i % 7) - 2;
    p.mass = 0.5f + static_cast<float>(i % 4);
    p.id = 1000 + i;
    std::snprintf(p.name, sizeof p.name, "p%u", i);
    p.color[0] = 0.1f * static_cast<float>(i % 10);
    p.color[3] = 1;
    p.spawned_at = i * 0.25;
    return p;
}

inline bool same(const Particle& a, const Particle& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.vx == b.vx && a.vy == b.vy &&
           a.vz == b.vz && a.mass == b.mass && a.id == b.id &&
           std::memcmp(a.name, b.name, sizeof a.name) == 0 &&
           std::memcmp(a.color, b.color, sizeof a.color) == 0 && a.spawned_at == b.spawned_at;
}
