// The array-of-structs version. You won't need to change this file.
#include "particles.hpp"

void ParticlesAoS::add(const Particle& p) {
    ps_.push_back(p);
}

std::size_t ParticlesAoS::size() const {
    return ps_.size();
}

const Particle& ParticlesAoS::get(std::size_t i) const {
    return ps_[i];
}

void ParticlesAoS::step(float dt) {
    for (Particle& p : ps_) {
        p.vz -= gravity * dt;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
    }
}

double ParticlesAoS::kinetic_energy() const {
    double total = 0;
    for (const Particle& p : ps_) {
        total += 0.5 * p.mass * (p.vx * p.vx + p.vy * p.vy + p.vz * p.vz);
    }
    return total;
}

std::size_t ParticlesAoS::count_below(float height) const {
    std::size_t n = 0;
    for (const Particle& p : ps_) {
        n += p.z < height;
    }
    return n;
}
