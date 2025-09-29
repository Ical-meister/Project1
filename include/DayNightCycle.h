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
        currentTime = fmod(glfwGetTime(), dayLength) / dayLength;
        float angle = currentTime * glm::two_pi<float>();

        // sun moves east->overhead->west->under ground
        direction = glm::normalize(glm::vec3(cos(angle), sin(angle), 0.0f));

        // daylight factor (sun height)
        float daylight = glm::clamp((direction.y + 0.4f) / 0.8f, 0.0f, 1.0f);
        daylight = daylight * daylight * (3.0f - 2.0f * daylight); // smoothstep easing

        // colors
        glm::vec3 dayColor = glm::vec3(1.0f, 1.0f, 0.9f);   // bright neutral
        glm::vec3 twilightColor = glm::vec3(1.0f, 0.5f, 0.3f);   // warm orange
        glm::vec3 nightColor = glm::vec3(0.1f, 0.1f, 0.3f);   // deep blue
        glm::vec3 dayToTwilight = glm::mix(dayColor, twilightColor, 1.0f - daylight);
        glm::vec3 twilightToNight = glm::mix(twilightColor, nightColor, 1.0f - daylight);

        // final diffuse/ambient color = weighted
        if (direction.y > 0.0f) {
            diffuse = dayToTwilight;   // transitioning day → twilight
        }
        else {
            diffuse = twilightToNight; // transitioning twilight → night
        }

        //if (direction.y > 0.2f) {
        //    diffuse = dayColor;
        //}
        //else if (direction.y > -0.2f) {
        //    // mix twilight with night/day depending on side
        //    float t = (direction.y + 0.2f) / 0.4f;
        //    diffuse = glm::mix(nightColor, twilightColor, t);
        //}
        //else {
        //    diffuse = nightColor;
        //}

        ambient = diffuse * 0.2f;
        specular = diffuse;
        backgroundColor = glm::mix(nightColor, diffuse, daylight);
    }
};
