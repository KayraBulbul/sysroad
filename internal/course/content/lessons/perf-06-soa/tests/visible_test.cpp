#include "doctest.h"

#include "helpers.hpp"

TEST_CASE("add, size and get") {
    ParticlesSoA soa;
    for (std::uint32_t i = 0; i < 3; ++i) {
        soa.add(sample_particle(i));
    }
    REQUIRE(soa.size() == 3);
    CHECK(soa.get(1).x == sample_particle(1).x);
    CHECK(soa.get(2).id == sample_particle(2).id);
    CHECK(same(soa.get(0), sample_particle(0)));
}

TEST_CASE("the same answers as the array of structs") {
    ParticlesAoS aos;
    ParticlesSoA soa;
    for (std::uint32_t i = 0; i < 100; ++i) {
        aos.add(sample_particle(i));
        soa.add(sample_particle(i));
    }
    CHECK(soa.kinetic_energy() == doctest::Approx(aos.kinetic_energy()));
    CHECK(soa.count_below(10) == aos.count_below(10));
    aos.step(0.5f);
    soa.step(0.5f);
    CHECK(soa.get(42).z == aos.get(42).z);
}
