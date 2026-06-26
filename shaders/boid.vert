#version 330 core

layout(location = 0) in vec2 aLocalPos;
layout(location = 1) in vec2 aInstancePos;
layout(location = 2) in vec2 aInstanceDir;
layout(location = 3) in vec4 aInstanceColor;

uniform mat4 uProjection;

out vec4 vColor;

// Rota el eje local +Y (punta del triángulo) hacia la dirección del boid.
mat2 rotationMatrix(vec2 dir) {
    dir = normalize(dir);
    return mat2(vec2(-dir.y, dir.x), vec2(dir.x, dir.y));
}

void main() {
    vec2 rotated = rotationMatrix(normalize(aInstanceDir)) * aLocalPos;
    vec2 worldPos = rotated + aInstancePos;
    gl_Position = uProjection * vec4(worldPos, 0.0, 1.0);
    vColor = aInstanceColor;
}
