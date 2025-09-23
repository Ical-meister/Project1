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
#include "Shader.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ============================================================================
// INPUT HANDLING
// ============================================================================

Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));  // Main camera
CameraMode currentCameraMode = FIRST_PERSON;  // Current interaction mode

// Display options
bool showGrid = true;
bool showOrigin = true;
bool mouseControlEnabled = false;

// Mouse input state
bool firstMouse = true;
float lastX = 600.0f;
float lastY = 450.0f;

// Keyboard input state (for continuous movement)
bool keys[1024] = { false };

// Keyboard input callback - handles discrete key presses
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		std::cout << "Key pressed: " << key << std::endl;
        switch (key) {
            // Movement keys - set state for continuous processing
        case GLFW_KEY_W:
            keys[GLFW_KEY_W] = (action != GLFW_RELEASE);
            break;
        case GLFW_KEY_S:
            keys[GLFW_KEY_S] = (action != GLFW_RELEASE);
            break;
        case GLFW_KEY_A:
            keys[GLFW_KEY_A] = (action != GLFW_RELEASE);
            break;
        case GLFW_KEY_D:
            keys[GLFW_KEY_D] = (action != GLFW_RELEASE);
            break;
        case GLFW_KEY_Q:
            keys[GLFW_KEY_Q] = (action != GLFW_RELEASE);
            break;
        case GLFW_KEY_E:
            keys[GLFW_KEY_E] = (action != GLFW_RELEASE);
            break;

            // Mouse control toggle
        case GLFW_KEY_M:
            if (action == GLFW_PRESS) {
                mouseControlEnabled = !mouseControlEnabled;
                if (mouseControlEnabled) {
                    // Enable mouse capture for look-around
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    // Release mouse cursor
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
            break;

            // Camera mode cycling
        case GLFW_KEY_C:
            if (action == GLFW_PRESS) {
                currentCameraMode = static_cast<CameraMode>((currentCameraMode + 1) % 3);
            }
            break;

            // Display toggles
        case GLFW_KEY_G:
            if (action == GLFW_PRESS) showGrid = !showGrid;
            break;
        case GLFW_KEY_O:
            if (action == GLFW_PRESS) showOrigin = !showOrigin;
            break;

            // Reset camera to default state
        case GLFW_KEY_R:
            if (action == GLFW_PRESS) {
                camera = Camera(glm::vec3(0.0f, 2.0f, 5.0f));
                currentCameraMode = FIRST_PERSON;
                mouseControlEnabled = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            break;

            // Exit application
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        }
    }
    else if (action == GLFW_RELEASE) {
        // Clear key state when released
        keys[key] = false;
    }
}

// ========================================================================
// MOUSE INPUT HANDLING - Core of mouse-based camera rotation
// ========================================================================

// Mouse movement callback - processes raw mouse input for camera rotation
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseControlEnabled) return;  // Only process when mouse control is active

    // Handle first mouse input to prevent camera jump
    // When mouse is first captured, we don't want sudden movement
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    // Calculate mouse movement delta from last frame
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Y is reversed (screen vs world coords)

    // Update last position for next frame
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    // Process the movement through camera system
    camera.processMouseMovement(xoffset, yoffset);
}

// Mouse scroll callback - handles zoom and orbit distance
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (currentCameraMode == ORBIT_CAMERA) {
        // In orbit mode, scroll controls distance from target
        camera.orbitDistance -= static_cast<float>(yoffset);
        camera.orbitDistance = glm::clamp(camera.orbitDistance, 1.0f, 20.0f);
    }
    else {
        // In other modes, scroll controls field of view (zoom)
        camera.processMouseScroll(static_cast<float>(yoffset));
    }
}

// ========================================================================
// CONTINUOUS INPUT PROCESSING - WASD movement implementation
// ========================================================================

// Process continuous keyboard input for smooth movement
// Called every frame to handle held keys
void processInput(float deltaTime) {
    // Forward/backward movement
    if (keys[GLFW_KEY_W])
        camera.processKeyboard(0, deltaTime); // FORWARD
    if (keys[GLFW_KEY_S])
        camera.processKeyboard(1, deltaTime); // BACKWARD

    // Left/right strafing
    if (keys[GLFW_KEY_A])
        camera.processKeyboard(2, deltaTime); // LEFT
    if (keys[GLFW_KEY_D])
        camera.processKeyboard(3, deltaTime); // RIGHT

    // Vertical movement
    if (keys[GLFW_KEY_Q])
        camera.processKeyboard(4, deltaTime); // UP
    if (keys[GLFW_KEY_E])
        camera.processKeyboard(5, deltaTime); // DOWN
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
    Model tree("assets/models/CommonTree_1.obj");

	// 6. Setup input callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 7. Main render loop
    while (!glfwWindowShouldClose(window)) {

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // dark blue background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader
        shader.use();

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
