#include "Renderer.h"
#include "Simulation.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <string>

namespace {
constexpr int kInitialWidth = 1280;
constexpr int kInitialHeight = 720;

bool initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        return false;
    }
    return true;
}

void drawControlPanel(Simulation& simulation) {
    SimulationParams params = simulation.params();

    ImGui::Begin("Controles - Boids");
    ImGui::Text("Laboratorio 10: Comportamiento Colectivo");
    ImGui::Separator();

    if (ImGui::SliderInt("Numero de Boids", &params.boidCount, 10, 500)) {
        simulation.setParams(params);
    }

    ImGui::SliderFloat("Distancia de vecindad", &params.neighborDistance, 20.0f, 200.0f, "%.1f");
    ImGui::SliderFloat("Distancia de separacion", &params.separationDistance, 5.0f, 80.0f, "%.1f");
    ImGui::SliderFloat("Intensidad separacion", &params.separationWeight, 0.0f, 4.0f, "%.2f");
    ImGui::SliderFloat("Intensidad alineamiento", &params.alignmentWeight, 0.0f, 4.0f, "%.2f");
    ImGui::SliderFloat("Intensidad cohesion", &params.cohesionWeight, 0.0f, 4.0f, "%.2f");

    ImGui::Separator();
    ImGui::SliderFloat("Velocidad", &params.speed, kMinBoidSpeed, 500.0f, "%.1f");

    const char* edgeModes[] = {"Rebote en bordes", "Mundo toroidal"};
    int edgeModeIndex = static_cast<int>(params.edgeMode);
    if (ImGui::Combo("Comportamiento en bordes", &edgeModeIndex, edgeModes, 2)) {
        params.edgeMode = static_cast<EdgeMode>(edgeModeIndex);
    }

    if (ImGui::Button("Reiniciar poblacion")) {
        simulation.setParams(params);
        simulation.reset();
    }

    simulation.setParams(params);

    ImGui::Separator();
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Pausa: Espacio | Salir: ESC");
    ImGui::End();
}
}  // namespace

int main() {
    if (!glfwInit()) {
        std::cerr << "No se pudo inicializar GLFW\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(kInitialWidth, kInitialHeight, "Lab 10 - Simulacion Boids", nullptr, nullptr);
    if (!window) {
        std::cerr << "No se pudo crear la ventana GLFW\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "No se pudo cargar OpenGL con GLAD\n";
        glfwTerminate();
        return 1;
    }

    Simulation simulation;
    Renderer renderer;

    simulation.setWorldSize(static_cast<float>(kInitialWidth), static_cast<float>(kInitialHeight));
    simulation.reset();

    if (!renderer.init()) {
        std::cerr << "Fallo la inicializacion del renderer\n";
        glfwTerminate();
        return 1;
    }

    renderer.resize(kInitialWidth, kInitialHeight);

    if (!initImGui(window)) {
        std::cerr << "Fallo la inicializacion de ImGui\n";
        renderer.shutdown();
        glfwTerminate();
        return 1;
    }

    glfwSetWindowUserPointer(window, &renderer);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
        auto* rendererPtr = static_cast<Renderer*>(glfwGetWindowUserPointer(win));
        if (rendererPtr != nullptr) {
            rendererPtr->resize(w, h);
        }
    });

    bool paused = false;
    bool spaceWasPressed = false;
    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!spaceWasPressed) {
                paused = !paused;
                spaceWasPressed = true;
            }
        } else {
            spaceWasPressed = false;
        }

        const double currentTime = glfwGetTime();
        const float dt = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        simulation.setWorldSize(static_cast<float>(width), static_cast<float>(height));

        if (!paused) {
            simulation.update(std::min(dt, 0.033f));
        }

        renderer.draw(simulation);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        drawControlPanel(simulation);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
