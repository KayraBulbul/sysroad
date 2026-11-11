#include "particles.hpp"

#include <cstring>

void ParticlesSoA::add(const Particle& p) {
    // TODO: push each field onto its own vector.
    (void)p;
}

std::size_t ParticlesSoA::size() const {
    // TODO
    return 0;
}

Particle ParticlesSoA::get(std::size_t i) const {
    // TODO
    (void)i;
    return {};
}

void ParticlesSoA::step(float dt) {
    // TODO: the same arithmetic as ParticlesAoS::step, in aos.cpp.
    (void)dt;
}

double ParticlesSoA::kinetic_energy() const {
    // TODO
    return 0;
}

std::size_t ParticlesSoA::count_below(float height) const {
    // TODO
    (void)height;
    return 0;
}
