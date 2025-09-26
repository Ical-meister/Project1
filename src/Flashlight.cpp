#include <glm/glm.hpp>

struct Flashlight {
    bool enabled;
    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float cutOff;       // inner cutoff (cosine of angle)
    float outerCutOff;  // outer cutoff (cosine of angle)
    float constant;
    float linear;
    float quadratic;

    Flashlight()
        : enabled(false),
        position(0.0f), direction(0.0f, 0.0f, -1.0f),
        ambient(0.1f), diffuse(1.0f), specular(1.0f),
        cutOff(glm::cos(glm::radians(12.5f))),
        outerCutOff(glm::cos(glm::radians(17.5f))),
        constant(1.0f), linear(0.09f), quadratic(0.032f)
    {
    }

    void toggle() { enabled = !enabled; }

    void updateFromCamera(const glm::vec3& camPos, const glm::vec3& camFront) {
        position = camPos;
        direction = glm::normalize(camFront);
    }
};
