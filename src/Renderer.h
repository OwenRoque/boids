#pragma once

#include "Simulation.h"

#include <string>
#include <vector>

#include <glm/glm.hpp>

class Renderer {
public:
    bool init();
    void shutdown();
    void resize(int width, int height);
    void draw(const Simulation& simulation);

private:
    bool loadShaderProgram();
    std::string readTextFile(const char* path) const;
    void rebuildInstanceBuffers(const std::vector<Boid>& boids);

    unsigned int shaderProgram_ = 0;
    unsigned int vao_ = 0;
    unsigned int vboShape_ = 0;
    unsigned int vboInstance_ = 0;
    int projectionLocation_ = -1;
    glm::mat4 projection_{1.0f};
    int worldWidth_ = 1;
    int worldHeight_ = 1;
    size_t instanceCapacity_ = 0;
};
