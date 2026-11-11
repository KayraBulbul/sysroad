#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <vector>

// One particle, as a game or a simulation might store it. The physics
// reads the first eight fields every frame. The rest is for drawing it and
// debugging, and the physics never touches them.
struct Particle {
    float x, y, z;     // position, in metres
    float vx, vy, vz;  // velocity, in metres per second
    float mass;        // kilograms
    std::uint32_t id;
    char name[24];
    float color[4];
    double spawned_at;
};

constexpr float gravity = 9.81f;

// Array of structs: one Particle after another. Already written, in aos.cpp.
class ParticlesAoS {
  public:
    void add(const Particle& p);
    std::size_t size() const;
    const Particle& get(std::size_t i) const;

    // Moves every particle forward dt seconds: gravity pulls vz down by
    // gravity * dt, then each position moves by its velocity * dt.
    void step(float dt);
    // The sum of ½ m v² over all particles, added up in index order.
    double kinetic_energy() const;
    // How many particles have z below height.
    std::size_t count_below(float height) const;

  private:
    std::vector<Particle> ps_;
};

// Struct of arrays: the same particles, with one vector per hot field and
// the cold fields kept together out of the way. Yours, in soa.cpp.
class ParticlesSoA {
  public:
    void add(const Particle& p);
    std::size_t size() const;
    // Puts particle i back together, cold fields included.
    Particle get(std::size_t i) const;

    void step(float dt);
    double kinetic_energy() const;
    std::size_t count_below(float height) const;

  private:
    struct Cold {
        char name[24];
        float color[4];
        double spawned_at;
    };

    std::vector<float> x_, y_, z_;
    std::vector<float> vx_, vy_, vz_;
    std::vector<float> mass_;
    std::vector<std::uint32_t> id_;
    std::vector<Cold> cold_;
};
