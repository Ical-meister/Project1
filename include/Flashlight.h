#pragma once
#include <glm/glm.hpp>

struct Flashlight {
    bool enabled = false;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);

    glm::vec3 ambient = glm::vec3(0.0);
    glm::vec3 diffuse = glm::vec3(0.8f, 0.75f, 0.6f);
    glm::vec3 specular = glm::vec3(1.0f, 0.9f, 0.6f);

    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(17.5f));

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    void toggle() { enabled = !enabled; }
    void updateFromCamera(const glm::vec3& camPos, const glm::vec3& camFront) {
        position = camPos;
        direction = glm::normalize(camFront);
    }
};