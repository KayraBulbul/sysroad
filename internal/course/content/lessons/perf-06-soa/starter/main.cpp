// Races the two layouts over a million particles. You won't need to change
// this file.
#include <cstdio>
#include <cstring>
#include <functional>

#include "bench.hpp"
#include "particles.hpp"

static Particle make(std::uint32_t i) {
    Particle p{};
    // Spread them about with cheap arithmetic that's the same everywhere.
    p.x = static_cast<float>(i % 1000);
    p.y = static_cast<float>(i / 1000 % 1000);
    p.z = static_cast<float>(i * 7919 % 100);
    p.vx = static_cast<float>(i % 13) - 6;
    p.vy = static_cast<float>(i % 7) - 3;
    p.vz = static_cast<float>(i % 11);
    p.mass = 1 + static_cast<float>(i % 5);
    p.id = i;
    std::snprintf(p.name, sizeof p.name, "particle-%u", i);
    return p;
}

static double best_ms(const std::function<void()>& f) {
    return measure(f, 5, 1e6).min / 1e6;
}

static void race(const char* what, double aos, double soa) {
    std::printf("%-22s %8.2f ms %8.2f ms   %.1fx\n", what, aos, soa, aos / soa);
}

int main() {
    const std::uint32_t n = 1'000'000;
    ParticlesAoS aos;
    ParticlesSoA soa;
    for (std::uint32_t i = 0; i < n; ++i) {
        aos.add(make(i));
        soa.add(make(i));
    }
    std::printf("%zu particles, %zu bytes each as a struct\n\n", aos.size(), sizeof(Particle));
    if (soa.size() != aos.size() || soa.count_below(50) != aos.count_below(50)) {
        std::printf("Your ParticlesSoA doesn't hold the same particles yet, so there's\n"
                    "nothing to race. The tests below say what's wrong.\n");
        return 0;
    }
    std::printf("%-22s %11s %11s\n", "", "AoS", "SoA");

    race("step", best_ms([&] { aos.step(0.001f); }), best_ms([&] { soa.step(0.001f); }));
    race("kinetic_energy", best_ms([&] { do_not_optimize(aos.kinetic_energy()); }),
         best_ms([&] { do_not_optimize(soa.kinetic_energy()); }));
    race("count_below", best_ms([&] { do_not_optimize(aos.count_below(50)); }),
         best_ms([&] { do_not_optimize(soa.count_below(50)); }));
}
