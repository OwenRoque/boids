#include "Boid.h"

#include <algorithm>
#include <cmath>

namespace {
float length(const glm::vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
}  // namespace

void Boid::syncDirection() {
    const float speed = length(velocity);
    if (speed > 1e-5f) {
        direction = velocity / speed;
    }
}

void Boid::applyAcceleration(const glm::vec2& acceleration, float dt) {
    velocity += acceleration * dt;
    position += velocity * dt;
    syncDirection();
}

void Boid::clampSpeed(float minSpeed, float maxSpeed) {
    const float speed = length(velocity);
    if (speed < 1e-5f) {
        velocity = direction * minSpeed;
        return;
    }

    const float clamped = std::clamp(speed, minSpeed, maxSpeed);
    velocity = (velocity / speed) * clamped;
    syncDirection();
}
