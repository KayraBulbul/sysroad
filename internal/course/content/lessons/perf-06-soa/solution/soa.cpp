#include "particles.hpp"

#include <cstring>

void ParticlesSoA::add(const Particle& p) {
    x_.push_back(p.x);
    y_.push_back(p.y);
    z_.push_back(p.z);
    vx_.push_back(p.vx);
    vy_.push_back(p.vy);
    vz_.push_back(p.vz);
    mass_.push_back(p.mass);
    id_.push_back(p.id);
    Cold c;
    std::memcpy(c.name, p.name, sizeof c.name);
    std::memcpy(c.color, p.color, sizeof c.color);
    c.spawned_at = p.spawned_at;
    cold_.push_back(c);
}

std::size_t ParticlesSoA::size() const {
    return x_.size();
}

Particle ParticlesSoA::get(std::size_t i) const {
    Particle p;
    p.x = x_[i];
    p.y = y_[i];
    p.z = z_[i];
    p.vx = vx_[i];
    p.vy = vy_[i];
    p.vz = vz_[i];
    p.mass = mass_[i];
    p.id = id_[i];
    std::memcpy(p.name, cold_[i].name, sizeof p.name);
    std::memcpy(p.color, cold_[i].color, sizeof p.color);
    p.spawned_at = cold_[i].spawned_at;
    return p;
}

void ParticlesSoA::step(float dt) {
    std::size_t n = size();
    for (std::size_t i = 0; i < n; ++i) {
        vz_[i] -= gravity * dt;
        x_[i] += vx_[i] * dt;
        y_[i] += vy_[i] * dt;
        z_[i] += vz_[i] * dt;
    }
}

double ParticlesSoA::kinetic_energy() const {
    double total = 0;
    std::size_t n = size();
    for (std::size_t i = 0; i < n; ++i) {
        total += 0.5 * mass_[i] * (vx_[i] * vx_[i] + vy_[i] * vy_[i] + vz_[i] * vz_[i]);
    }
    return total;
}

std::size_t ParticlesSoA::count_below(float height) const {
    std::size_t count = 0;
    for (float z : z_) {
        count += z < height;
    }
    return count;
}
