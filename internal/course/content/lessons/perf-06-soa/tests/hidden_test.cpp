#include "doctest.h"

#include "helpers.hpp"

TEST_CASE("an empty set of particles") {
    ParticlesSoA soa;
    CHECK(soa.size() == 0);
    CHECK(soa.kinetic_energy() == 0);
    CHECK(soa.count_below(1e9f) == 0);
    soa.step(1);
}

TEST_CASE("get puts every field back, cold ones too") {
    ParticlesSoA soa;
    for (std::uint32_t i = 0; i < 50; ++i) {
        soa.add(sample_particle(i));
    }
    for (std::uint32_t i = 0; i < 50; ++i) {
        CAPTURE(i);
        CHECK(same(soa.get(i), sample_particle(i)));
    }
}

TEST_CASE("many steps match the array of structs exactly") {
    ParticlesAoS aos;
    ParticlesSoA soa;
    for (std::uint32_t i = 0; i < 300; ++i) {
        aos.add(sample_particle(i));
        soa.add(sample_particle(i));
    }
    for (int s = 0; s < 20; ++s) {
        aos.step(0.01f * static_cast<float>(s));
        soa.step(0.01f * static_cast<float>(s));
    }
    for (std::uint32_t i = 0; i < 300; ++i) {
        CAPTURE(i);
        CHECK(same(soa.get(i), aos.get(i)));
    }
    CHECK(soa.kinetic_energy() == doctest::Approx(aos.kinetic_energy()));
    for (float h : {-100.0f, 0.0f, 5.0f, 12.5f, 1000.0f}) {
        CAPTURE(h);
        CHECK(soa.count_below(h) == aos.count_below(h));
    }
}

TEST_CASE("step uses the new vz for z") {
    Particle p{};
    p.vz = 0;
    p.mass = 2;
    ParticlesSoA soa;
    soa.add(p);
    soa.step(1);
    CHECK(soa.get(0).vz == doctest::Approx(-gravity));
    CHECK(soa.get(0).z == doctest::Approx(-gravity));
    CHECK(soa.kinetic_energy() == doctest::Approx(0.5 * 2 * gravity * gravity));
}

TEST_CASE("count_below is strictly below") {
    Particle p{};
    p.z = 3;
    ParticlesSoA soa;
    soa.add(p);
    CHECK(soa.count_below(3) == 0);
    CHECK(soa.count_below(3.001f) == 1);
}
