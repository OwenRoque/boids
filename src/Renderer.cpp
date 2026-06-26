#include "Renderer.h"

#include <glad/glad.h>

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
struct InstanceData {
    glm::vec2 position;
    glm::vec2 direction;
    glm::vec4 color;
};

unsigned int compileShader(unsigned int type, const std::string& source) {
    const unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        throw std::runtime_error(std::string("Error compilando shader: ") + infoLog);
    }

    return shader;
}

unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        throw std::runtime_error(std::string("Error enlazando programa: ") + infoLog);
    }

    return program;
}
}  // namespace

bool Renderer::init() {
    if (!loadShaderProgram()) {
        return false;
    }

    const float triangleShape[] = {
        0.0f, 8.0f,
        -5.0f, -4.0f,
        5.0f, -4.0f};

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vboShape_);
    glGenBuffers(1, &vboInstance_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vboShape_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleShape), triangleShape, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, vboInstance_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);

    const GLsizei stride = static_cast<GLsizei>(sizeof(InstanceData));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(glm::vec2)));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(2 * sizeof(glm::vec2)));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Renderer::shutdown() {
    if (vboInstance_ != 0) {
        glDeleteBuffers(1, &vboInstance_);
        vboInstance_ = 0;
    }
    if (vboShape_ != 0) {
        glDeleteBuffers(1, &vboShape_);
        vboShape_ = 0;
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
}

void Renderer::resize(int width, int height) {
    worldWidth_ = std::max(width, 1);
    worldHeight_ = std::max(height, 1);
    glViewport(0, 0, worldWidth_, worldHeight_);
    projection_ = glm::ortho(0.0f, static_cast<float>(worldWidth_), static_cast<float>(worldHeight_), 0.0f, -1.0f, 1.0f);
}

void Renderer::draw(const Simulation& simulation) {
    glClearColor(0.06f, 0.08f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const auto& boids = simulation.boids();
    if (boids.empty()) {
        return;
    }

    rebuildInstanceBuffers(boids);

    glUseProgram(shaderProgram_);
    glUniformMatrix4fv(projectionLocation_, 1, GL_FALSE, &projection_[0][0]);

    glBindVertexArray(vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, static_cast<GLsizei>(boids.size()));
    glBindVertexArray(0);
}

bool Renderer::loadShaderProgram() {
    try {
        const std::string vertexSource = readTextFile("shaders/boid.vert");
        const std::string fragmentSource = readTextFile("shaders/boid.frag");

        const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        shaderProgram_ = linkProgram(vertexShader, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        projectionLocation_ = glGetUniformLocation(shaderProgram_, "uProjection");
        return true;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return false;
    }
}

std::string Renderer::readTextFile(const char* path) const {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error(std::string("No se pudo abrir shader: ") + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Renderer::rebuildInstanceBuffers(const std::vector<Boid>& boids) {
    std::vector<InstanceData> instances;
    instances.reserve(boids.size());

    for (size_t i = 0; i < boids.size(); ++i) {
        const Boid& boid = boids[i];
        const float hue = static_cast<float>(i % 12) / 12.0f;
        const glm::vec3 rgb = glm::vec3(
            0.55f + 0.35f * std::sin(hue * 6.28318f),
            0.65f + 0.25f * std::sin(hue * 6.28318f + 2.0f),
            0.85f + 0.10f * std::sin(hue * 6.28318f + 4.0f));

        instances.push_back({boid.position, boid.direction, glm::vec4(rgb, 0.92f)});
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboInstance_);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);
    instanceCapacity_ = instances.size();
}
