#pragma once

#include "Boid.h"

#include <vector>

enum class EdgeMode { Bounce, Toroidal };

struct SimulationParams {
    int boidCount = 80;
    float neighborDistance = 80.0f;
    float separationDistance = 25.0f;
    float separationWeight = 1.5f;
    float alignmentWeight = 1.0f;
    float cohesionWeight = 1.0f;
    float speed = 250.0f;
    float maxForce = 200.0f;
    EdgeMode edgeMode = EdgeMode::Toroidal;
};

constexpr float kMinBoidSpeed = 150.0f;

class Simulation {
public:
    void setWorldSize(float width, float height);
    void setParams(const SimulationParams& params);
    const SimulationParams& params() const { return params_; }
    const std::vector<Boid>& boids() const { return boids_; }

    void reset();
    void update(float dt);

private:
    glm::vec2 computeSeparation(const Boid& boid) const;
    glm::vec2 computeAlignment(const Boid& boid) const;
    glm::vec2 computeCohesion(const Boid& boid) const;
    glm::vec2 steerTowards(const glm::vec2& desired, const Boid& boid) const;
    void handleEdges(Boid& boid) const;

    SimulationParams params_{};
    std::vector<Boid> boids_;
    float worldWidth_ = 1280.0f;
    float worldHeight_ = 720.0f;
    int lastBoidCount_ = 0;
};
