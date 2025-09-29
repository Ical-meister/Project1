#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GLFW/glfw3.h>

struct DayNightCycle {
    float dayLength;     // seconds for a full cycle
    float currentTime;   // 0–1 normalized
	float daylight;	     // 0–1 normalized (sun height)

    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 backgroundColor;

    DayNightCycle(float length = 60.0f) : dayLength(length), currentTime(0.0f) {}

    void update() {
        // progress through the day (0 → 1)
        currentTime = fmod(glfwGetTime(), dayLength) / dayLength;

        // angle from 0 to 2π
        float angle = currentTime * glm::two_pi<float>();

        // sun rotates east (x) → up (y) → west (x) → down (y)
        direction = glm::normalize(glm::vec3(cos(angle), sin(angle), 0.0f));

        // raw daylight factor based on sun height
        float raw = direction.y;

        // twilight-aware factor
        daylight = glm::clamp((raw + 0.4f) / 0.8f, 0.0f, 1.0f);

        // use daylight for light colors
        glm::vec3 dayColor = glm::vec3(1.0f, 1.0f, 0.9f);
        glm::vec3 twilight = glm::vec3(1.0f, 0.5f, 0.3f);
        glm::vec3 nightColor = glm::vec3(0.05f, 0.05f, 0.2f);

        diffuse = glm::mix(nightColor, dayColor, daylight);
        ambient = diffuse * 0.2f;
        specular = diffuse;
        backgroundColor = glm::mix(nightColor, dayColor, daylight);
    }
};
