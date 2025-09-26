#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

struct DayNightCycle {
    float dayLength;      // seconds per full cycle
    float currentTime;    // [0,1] fraction of cycle

    glm::vec3 lightDir;
    glm::vec3 lightColor;
    float intensity;
    glm::vec3 backgroundColor;

    DayNightCycle(float length = 20.0f)
        : dayLength(length), currentTime(0.0f),
        lightDir(0.0f, 1.0f, 0.0f), lightColor(1.0f),
        intensity(1.0f), backgroundColor(0.0f) {
    }

    void update() {
        // normalized time in [0,1]
        currentTime = fmod(glfwGetTime(), dayLength) / dayLength;
        float angle = currentTime * 360.0f;

        // rotating east->west
        lightDir = glm::normalize(glm::vec3(
            cos(glm::radians(angle)),
            sin(glm::radians(angle)),
            0.0f
        ));

        // determine light color & intensity
        if (lightDir.y > 0.2f) { // Day
            lightColor = glm::vec3(1.0f, 1.0f, 0.9f);
            intensity = 1.0f;
        }
        else if (lightDir.y > 0.0f) { // Sunrise/sunset
            lightColor = glm::vec3(1.0f, 0.5f, 0.2f);
            intensity = 0.6f;
        }
        else { // Night
            lightColor = glm::vec3(0.1f, 0.1f, 0.3f);
            intensity = 0.2f;
        }

        // background color transitions
        backgroundColor = glm::mix(
            glm::vec3(0.02f, 0.02f, 0.08f), // night sky
            lightColor,
            glm::clamp(lightDir.y, 0.0f, 1.0f)
        );
    }
};