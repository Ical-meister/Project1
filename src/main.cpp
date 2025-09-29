#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"
#pragma message("Compiling with Model.h from: " __FILE__)

#include "Shader.h"
#include "Camera.h"
#include "Flashlight.h"
#include "DayNightCycle.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include <vector>
using std::vector;

// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.25f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Flashlight
Flashlight flashlight;
static bool pressed = false;

// Day-Night Cycle
DayNightCycle cycle(60.0f);

struct ObjectInstance {
    glm::vec3 position;
    glm::vec3 scale;
    float rotationDeg;
};

// Input handling
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    static bool pressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!pressed) {
            flashlight.toggle();
            pressed = true;
        }
    }
    else {
        pressed = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

// Ground vertices (X, Y, Z,  NormalX, NormalY, NormalZ,  U, V)
float groundVertices[] = {
    // positions           // normals         // texcoords
   -30.0f, 0.0f, -30.0f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    30.0f, 0.0f, -30.0f,   0.0f, 1.0f, 0.0f, 50.0f, 0.0f,
    30.0f, 0.0f,  30.0f,   0.0f, 1.0f, 0.0f, 50.0f, 50.0f,
   -30.0f, 0.0f,  30.0f,   0.0f, 1.0f, 0.0f,  0.0f, 50.0f
};

unsigned int groundIndices[] = {
    0, 1, 2,
    2, 3, 0
};

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

std::vector<ObjectInstance> generateInstances(
    int count,
    float minX, float maxX,
    float minZ, float maxZ,
    float minScale, float maxScale,
    unsigned int seed = 42   // default seed
) {
    std::vector<ObjectInstance> instances;

    std::mt19937 gen(seed);  // deterministic random numbers

    std::uniform_real_distribution<float> distX(minX, maxX);
    std::uniform_real_distribution<float> distZ(minZ, maxZ);
    std::uniform_real_distribution<float> distScale(minScale, maxScale);
    std::uniform_real_distribution<float> distRot(0.0f, 360.0f);

    for (int i = 0; i < count; i++) {
        ObjectInstance inst;
        inst.position = glm::vec3(distX(gen), 0.0f, distZ(gen));
        inst.scale = glm::vec3(distScale(gen));
        inst.rotationDeg = distRot(gen);

        instances.push_back(inst);
    }

    return instances;
}

// Format: Number of objects, min/max X,Z define world bounds, scale range, seed
std::vector<ObjectInstance> tree1Instances = generateInstances(
    60, -30.0f, 30.0f, -30.0f, 30.0f, 0.08f, 0.95f, 12345  );

std::vector<ObjectInstance> tree2Instances = generateInstances(
    30, -30.0f, 30.0f, -30.0f, 30.0f, 0.08f, 0.95f, 123456);

std::vector<ObjectInstance> rockInstances = generateInstances(
    30, -30.0f, 30.0f, -30.0f, 30.0f, 0.15f, 0.35f, 67890  );

std::vector<ObjectInstance> fernInstances = generateInstances(
    450, -30.0f, 30.0f, -30.0f, 30.0f, 0.05f, 0.1f, 333
);

std::vector<ObjectInstance> flower3_groupInstances = generateInstances(
    150, -30.0f, 30.0f, -30.0f, 30.0f, 0.03f, 0.05f, 444
);

std::vector<ObjectInstance> grassShortInstances = generateInstances(
    80, -30.0f, 30.0f, -30.0f, 30.0f, 0.03f, 0.05f, 9090
);

std::vector<ObjectInstance> farmHouseInstances = {
    { glm::vec3(5.0f, 0.0f, -10.0f), glm::vec3(0.1f), 15.0f }
};

unsigned int loadCubemap(const std::vector<std::string>& faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void drawInstance(Shader& shader, Model& model, const ObjectInstance& inst) {
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, inst.position);
    m = glm::rotate(m, glm::radians(inst.rotationDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, inst.scale);
    shader.setMat4("model", m);

    
    model.Draw(shader.ID);   
    
}

std::vector<ObjectInstance> forestWallInstances;

void generateForestWall(float halfSize, int countPerSide) {
    std::mt19937 rng(42); // fixed seed for consistency (change/remove for different runs)
    std::uniform_real_distribution<float> scaleDist(0.18f, 0.25f);
    std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);

    float spacing = (2.0f * halfSize) / (countPerSide - 1);

    // Z-edges
    for (int i = 0; i < countPerSide; i++) {
        float x = -halfSize + i * spacing;

        // near edge (-Z)
        forestWallInstances.push_back({
            glm::vec3(x, 0.0f, -halfSize),
            glm::vec3(scaleDist(rng)),
            rotDist(rng)
            });

        // far edge (+Z)
        forestWallInstances.push_back({
            glm::vec3(x, 0.0f, halfSize),
            glm::vec3(scaleDist(rng)),
            rotDist(rng)
            });
    }

    // X-edges
    for (int i = 0; i < countPerSide; i++) {
        float z = -halfSize + i * spacing;

        // left edge (-X)
        forestWallInstances.push_back({
            glm::vec3(-halfSize, 0.0f, z),
            glm::vec3(scaleDist(rng)),
            rotDist(rng)
            });

        // right edge (+X)
        forestWallInstances.push_back({
            glm::vec3(halfSize, 0.0f, z),
            glm::vec3(scaleDist(rng)),
            rotDist(rng)
            });
    }
}


void renderScene(Shader& shader, Model& tree, Model& tree2, Model& rock,
    Model& fern, Model& grassShort, Model& Flower_3_Group,
    Model& Pine4, Model& farmHouse,
    unsigned int groundVAO, unsigned int grassTexture)
{
    // Ground
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    // bind grass texture to unit 0 (the same one texture_diffuse1 expects)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    glBindVertexArray(groundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // tree1
    for (const auto& inst : tree1Instances) drawInstance(shader, tree, inst);

    // tree2
    for (const auto& inst : tree2Instances) drawInstance(shader, tree2, inst);

    // rocks
    for (const auto& inst : rockInstances) drawInstance(shader, rock, inst);

    // bushes
    for (const auto& inst : fernInstances) drawInstance(shader, fern, inst);

    // flowers
    for (const auto& inst : flower3_groupInstances) drawInstance(shader, Flower_3_Group, inst);

    // grass
    for (const auto& inst : grassShortInstances) drawInstance(shader, grassShort, inst);

    // farmhouse
    for (auto& inst : farmHouseInstances) drawInstance(shader, farmHouse, inst);

    // forest wall
    for (const auto& inst : forestWallInstances) drawInstance(shader, Pine4, inst);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 3. Initialize GLEW
    glewExperimental = GL_TRUE; // needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

  	// Set callbacks
    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable depth test (important for 3D rendering)
    glEnable(GL_DEPTH_TEST);

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Light space transformation matrix (for shadows)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 20.0f;
    lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
    lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), // light position
        glm::vec3(0.0f, 0.0f, 0.0f),   // look at center
        glm::vec3(0.0f, 1.0f, 0.0f));  // up vector
    lightSpaceMatrix = lightProjection * lightView;


    // 4. Load shaders
    Shader flashlightshader("shaders/basic.vs", "shaders/flashlight.fs");
    Shader shader("shaders/model_loading.vs", "shaders/model_loading.fs");
    shader.use();
    shader.setInt("texture_diffuse1", 0);
    shader.setInt("texture_specular1", 1);
    shader.setFloat("shininess", 32.0f);

    // Depth shader (renders scene from light's POV)
    Shader depthShader("shaders/depth_shader.vs", "shaders/depth_shader.fs");

    unsigned int groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);

    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Set projection (only once unless window size changes)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f, 100.0f);
    shader.setMat4("projection", projection);

    // 5. Load model with Assimp
    Model tree("assets/models/CommonTree_1/CommonTree_1.obj");
    Model tree2("assets/models/CommonTree_2/CommonTree_2.obj");
    Model rock("assets/models/Rock_Medium_1/Rock_Medium_1.obj");
    Model fern("assets/models/Fern_1/Fern_1.obj");
    Model grassShort("assets/models/Grass_Common_Short/Grass_Common_Short.obj");
    Model Flower_3_Group("assets/models/Flower_3_Group/Flower_3_Group.obj");
    Model Pine4("assets/models/Pine_4/Pine_4.obj");
    Model farmHouse("assets/models/farmhouse/farmhouse_obj.obj");

    generateForestWall(30.0f, 40); // 30 is halfSize since plane is -30 to +30

    // Skybox setup
    vector<std::string> faces = {
    "assets/skybox/px.png", // +X  right
    "assets/skybox/nx.png", // -X  left
    "assets/skybox/py.png", // +Y  top
    "assets/skybox/ny.png", // -Y  bottom
    "assets/skybox/pz.png", // +Z  front
    "assets/skybox/nz.png"  // -Z  back
    };

    unsigned int cubemapTexture = loadCubemap(faces);
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");

    unsigned int grassTexture;
    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    // set wrapping and filtering BEFORE/AFTER glTexImage2D (both work, but safer before mipmaps)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    

    int width, height, nrChannels;
    unsigned char* data = stbi_load("assets/textures/CartoonGrass.jpg", &width, &height, &nrChannels, STBI_rgb_alpha);

    if (!data) {
        std::cout << "Failed to load grass texture: " << stbi_failure_reason() << std::endl;
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    // 6. Main render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        cycle.update();

        // update sky background (if not using cubemap)
        glClearColor(cycle.backgroundColor.r,
            cycle.backgroundColor.g,
            cycle.backgroundColor.b,
            1.0f);

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setFloat("material.shininess", 32.0f);

        // set directional light from cycle
        shader.use();
        shader.setVec3("dirLight.direction", cycle.direction);
        shader.setVec3("dirLight.ambient", cycle.ambient);
        shader.setVec3("dirLight.diffuse", cycle.diffuse);
        shader.setVec3("dirLight.specular", cycle.specular);

        shader.setVec3("fogColor", cycle.backgroundColor);
        shader.setFloat("fogDensity", 0.04f);

        /*// --- Point light (glowing rock) ---
        shader.setVec3("pointLights[0].position", glm::vec3(2.0f, 0.5f, 2.0f));  // rock position
        shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLights[0].diffuse", 1.0f, 0.6f, 0.3f);   // warm orange glow
        shader.setVec3("pointLights[0].specular", 1.0f, 0.6f, 0.3f);
        shader.setFloat("pointLights[0].constant", 1.0f);
        shader.setFloat("pointLights[0].linear", 0.09f);
        shader.setFloat("pointLights[0].quadratic", 0.032f);*/

		// Disable point light #0
        shader.setVec3("pointLights[0].position", glm::vec3(0.0f));  // rock position
        shader.setVec3("pointLights[0].ambient", glm::vec3(0.0f));
        shader.setVec3("pointLights[0].diffuse", glm::vec3(0.0f));   // warm orange glow
        shader.setVec3("pointLights[0].specular", glm::vec3(0.0f));
        shader.setFloat("pointLights[0].constant", 1.0f);
        shader.setFloat("pointLights[0].linear", 0.09f);
        shader.setFloat("pointLights[0].quadratic", 0.032f);

        // Disable point light #1
        shader.setVec3("pointLights[1].position", glm::vec3(0.0f));
        shader.setVec3("pointLights[1].ambient", 0.0f, 0.0f, 0.0f);
        shader.setVec3("pointLights[1].diffuse", 0.0f, 0.0f, 0.0f);
        shader.setVec3("pointLights[1].specular", 0.0f, 0.0f, 0.0f);
        shader.setFloat("pointLights[1].constant", 1.0f);
        shader.setFloat("pointLights[1].linear", 0.09f);
        shader.setFloat("pointLights[1].quadratic", 0.032f);

		// Flashlight
        flashlight.updateFromCamera(camera.Position, camera.Front);

        shader.use();
        shader.setBool("flashlight.enabled", flashlight.enabled);
        shader.setVec3("flashlight.position", flashlight.position);
        shader.setVec3("flashlight.direction", flashlight.direction);
        shader.setVec3("flashlight.ambient", flashlight.ambient);
        shader.setVec3("flashlight.diffuse", flashlight.diffuse);
        shader.setVec3("flashlight.specular", flashlight.specular);
        shader.setFloat("flashlight.cutOff", flashlight.cutOff);
        shader.setFloat("flashlight.outerCutOff", flashlight.outerCutOff);
        shader.setFloat("flashlight.constant", flashlight.constant);
        shader.setFloat("flashlight.linear", flashlight.linear);
        shader.setFloat("flashlight.quadratic", flashlight.quadratic);

        // Camera matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);

        // send light-space matrix to shader
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // bind shadow map texture to unit 2
        shader.setInt("shadowMap", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        // 1. Render depth map from light�s POV
        glViewport(0, 0, 1024, 1024);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        renderScene(depthShader, tree, tree2, rock, fern, grassShort, Flower_3_Group, Pine4, farmHouse, groundVAO, grassTexture);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Reset viewport and render scene normally
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setInt("shadowMap", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        renderScene(shader, tree, tree2, rock, fern, grassShort, Flower_3_Group, Pine4, farmHouse, groundVAO, grassTexture);


        // Draw skybox (last)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setVec3("skyTint", cycle.diffuse);
        skyboxShader.setFloat("tintStrength", 1.0f);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // reset to default

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();
    return 0;
}
