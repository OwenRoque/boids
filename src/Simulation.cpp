#include "Simulation.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace {
float length(const glm::vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

glm::vec2 limit(const glm::vec2& v, float maxLen) {
    const float len = length(v);
    if (len > maxLen && len > 1e-5f) {
        return (v / len) * maxLen;
    }
    return v;
}

float randomRange(std::mt19937& rng, float minValue, float maxValue) {
    std::uniform_real_distribution<float> dist(minValue, maxValue);
    return dist(rng);
}
}  // namespace

void Simulation::setWorldSize(float width, float height) {
    worldWidth_ = std::max(width, 1.0f);
    worldHeight_ = std::max(height, 1.0f);
}

void Simulation::setParams(const SimulationParams& params) {
    params_ = params;
    params_.boidCount = std::max(params_.boidCount, 1);
    params_.speed = std::max(params_.speed, kMinBoidSpeed);

    if (params_.boidCount != lastBoidCount_) {
        reset();
    }
}

void Simulation::reset() {
    static std::mt19937 rng{std::random_device{}()};

    boids_.clear();
    boids_.reserve(static_cast<size_t>(params_.boidCount));

    const float margin = 30.0f;
    for (int i = 0; i < params_.boidCount; ++i) {
        Boid boid;
        boid.position = {
            randomRange(rng, margin, worldWidth_ - margin),
            randomRange(rng, margin, worldHeight_ - margin)};

        const float angle = randomRange(rng, 0.0f, 6.2831853f);
        const float speed = randomRange(rng, kMinBoidSpeed, params_.speed);
        boid.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
        boid.syncDirection();
        boids_.push_back(boid);
    }

    lastBoidCount_ = params_.boidCount;
}

void Simulation::update(float dt) {
    if (boids_.empty()) {
        return;
    }

    std::vector<glm::vec2> accelerations(boids_.size(), glm::vec2{0.0f});

    for (size_t i = 0; i < boids_.size(); ++i) {
        const Boid& boid = boids_[i];

        const glm::vec2 separation = computeSeparation(boid) * params_.separationWeight;
        const glm::vec2 alignment = computeAlignment(boid) * params_.alignmentWeight;
        const glm::vec2 cohesion = computeCohesion(boid) * params_.cohesionWeight;

        accelerations[i] = separation + alignment + cohesion;
    }

    for (size_t i = 0; i < boids_.size(); ++i) {
        Boid& boid = boids_[i];
        boid.applyAcceleration(accelerations[i], dt);
        boid.clampSpeed(kMinBoidSpeed, params_.speed);
        handleEdges(boid);
        boid.syncDirection();
    }
}

glm::vec2 Simulation::computeSeparation(const Boid& boid) const {
    glm::vec2 steer{0.0f};
    int count = 0;

    for (const Boid& other : boids_) {
        if (&other == &boid) {
            continue;
        }

        const glm::vec2 offset = boid.position - other.position;
        const float distance = length(offset);
        if (distance > 0.0f && distance < params_.separationDistance) {
            steer += offset / distance / distance;
            ++count;
        }
    }

    if (count > 0) {
        steer /= static_cast<float>(count);
    }

    if (length(steer) > 0.0f) {
        steer = steer / length(steer) * params_.speed;
        return steerTowards(steer, boid);
    }

    return glm::vec2{0.0f};
}

glm::vec2 Simulation::computeAlignment(const Boid& boid) const {
    glm::vec2 averageVelocity{0.0f};
    int count = 0;

    for (const Boid& other : boids_) {
        if (&other == &boid) {
            continue;
        }

        const float distance = length(other.position - boid.position);
        if (distance > 0.0f && distance < params_.neighborDistance) {
            averageVelocity += other.velocity;
            ++count;
        }
    }

    if (count == 0) {
        return glm::vec2{0.0f};
    }

    averageVelocity /= static_cast<float>(count);
    if (length(averageVelocity) > 0.0f) {
        averageVelocity = averageVelocity / length(averageVelocity) * params_.speed;
        return steerTowards(averageVelocity, boid);
    }

    return glm::vec2{0.0f};
}

glm::vec2 Simulation::computeCohesion(const Boid& boid) const {
    glm::vec2 centerOfMass{0.0f};
    int count = 0;

    for (const Boid& other : boids_) {
        if (&other == &boid) {
            continue;
        }

        const float distance = length(other.position - boid.position);
        if (distance > 0.0f && distance < params_.neighborDistance) {
            centerOfMass += other.position;
            ++count;
        }
    }

    if (count == 0) {
        return glm::vec2{0.0f};
    }

    centerOfMass /= static_cast<float>(count);
    glm::vec2 desired = centerOfMass - boid.position;
    if (length(desired) > 0.0f) {
        desired = desired / length(desired) * params_.speed;
        return steerTowards(desired, boid);
    }

    return glm::vec2{0.0f};
}

glm::vec2 Simulation::steerTowards(const glm::vec2& desired, const Boid& boid) const {
    glm::vec2 steer = desired - boid.velocity;
    return limit(steer, params_.maxForce);
}

void Simulation::handleEdges(Boid& boid) const {
    if (params_.edgeMode == EdgeMode::Toroidal) {
        if (boid.position.x < 0.0f) {
            boid.position.x += worldWidth_;
        } else if (boid.position.x > worldWidth_) {
            boid.position.x -= worldWidth_;
        }

        if (boid.position.y < 0.0f) {
            boid.position.y += worldHeight_;
        } else if (boid.position.y > worldHeight_) {
            boid.position.y -= worldHeight_;
        }
        return;
    }

    const float margin = 20.0f;
    const float turnFactor = 3.0f;

    if (boid.position.x < margin) {
        boid.velocity.x += turnFactor * (margin - boid.position.x);
    } else if (boid.position.x > worldWidth_ - margin) {
        boid.velocity.x -= turnFactor * (boid.position.x - (worldWidth_ - margin));
    }

    if (boid.position.y < margin) {
        boid.velocity.y += turnFactor * (margin - boid.position.y);
    } else if (boid.position.y > worldHeight_ - margin) {
        boid.velocity.y -= turnFactor * (boid.position.y - (worldHeight_ - margin));
    }

    if (boid.position.x < 0.0f || boid.position.x > worldWidth_) {
        boid.velocity.x *= -1.0f;
        boid.position.x = std::clamp(boid.position.x, 0.0f, worldWidth_);
    }

    if (boid.position.y < 0.0f || boid.position.y > worldHeight_) {
        boid.velocity.y *= -1.0f;
        boid.position.y = std::clamp(boid.position.y, 0.0f, worldHeight_);
    }
}
