#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"
#pragma message("Compiling with Model.h from: " __FILE__)
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Input handling (basic escape key to close)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Assimp Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 3. Initialize GLEW
    glewExperimental = GL_TRUE; // needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Enable depth test (important for 3D rendering)
    glEnable(GL_DEPTH_TEST);

    // 4. Load shaders
    Shader shader("shaders/model_loading.vs", "shaders/model_loading.fs");
    shader.use();

    // Set projection (only once unless window size changes)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f, 100.0f);
    shader.setMat4("projection", projection);

    // 5. Load model with Assimp
    Model tree("assets/models/CommonTree_1/CommonTree_1.obj");




    // 6. Main render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader
        shader.use();

        // Set light properties (directional light)
        shader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
        shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // Pass camera position each frame (inside render loop)
        shader.setVec3("viewPos", glm::vec3(0.0f, 2.0f, 5.0f));

        // TODO: add camera/projection matrix setup here
        // glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // shader.setMat4("projection", projection);
        // shader.setMat4("view", view);

          // Update view every frame
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f),   // camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // look at origin
            glm::vec3(0.0f, 1.0f, 0.0f));  // up vector
        shader.setMat4("view", view);


        // Model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f)); // shrink if too large
        shader.setMat4("model", model);

        // Draw tree
        tree.Draw(shader.ID);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();
    return 0;
}
