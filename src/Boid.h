#pragma once

#include <glm/glm.hpp>

// Agente autónomo del modelo Boids (Reynolds, 1986).
struct Boid {
    glm::vec2 position{0.0f};
    glm::vec2 velocity{0.0f};
    glm::vec2 direction{1.0f, 0.0f};

    void syncDirection();
    void applyAcceleration(const glm::vec2& acceleration, float dt);
    void clampSpeed(float minSpeed, float maxSpeed);
};
