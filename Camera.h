#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

const float PI = 3.14159265358979323846f;

// ============================================================================
// CAMERA SYSTEM DEFINITIONS
// ============================================================================

// Camera modes define different interaction paradigms
enum CameraMode {
    FIRST_PERSON,  // FPS-style: camera moves freely, looks with mouse
    ORBIT_CAMERA,  // Orbit around target: mouse controls angles, scroll controls distance
    FREE_FLY       // Unrestricted 3D movement: like space flight
};

// Camera structure encapsulates all camera state and behavior
struct Camera {
    // === POSITION AND ORIENTATION ===
    glm::vec3 position;    // Camera location in world space
    glm::vec3 front;       // Direction camera is pointing (normalized)
    glm::vec3 up;          // Camera's up direction (normalized)
    glm::vec3 right;       // Camera's right direction (normalized)
    glm::vec3 worldUp;     // World's up direction (usually Y-axis)

    // === EULER ANGLES ===
    // Used to calculate front, right, and up vectors
    float yaw;             // Rotation around Y-axis (left/right)
    float pitch;           // Rotation around X-axis (up/down)

    // === CAMERA PROPERTIES ===
    float movementSpeed;   // Units per second for WASD movement
    float mouseSensitivity;// Mouse input scaling factor
    float fov;             // Field of view in degrees

    // === ORBIT CAMERA SPECIFIC ===
    glm::vec3 target;      // Point to orbit around
    float orbitDistance;   // Distance from target

    // Constructor: Initialize camera with sensible defaults
    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f)) {
        position = pos;
        worldUp = glm::vec3(0.0f, 1.0f, 0.0f);  // Y-axis up
        yaw = -90.0f;         // Start looking down negative Z-axis
        pitch = 0.0f;         // Start looking horizontally
        front = glm::vec3(0.0f, 0.0f, -1.0f);  // Initial forward direction
        movementSpeed = 2.5f;
        mouseSensitivity = 0.1f;
        fov = 45.0f;          // Standard FOV
        target = glm::vec3(0.0f);  // Default orbit target at origin
        orbitDistance = 5.0f;
        updateCameraVectors(); // Calculate initial right and up vectors
    }

    // ========================================================================
    // VIEW MATRIX CREATION - Core of world-to-view transformation
    // ========================================================================

    // Standard view matrix for first-person and free-fly modes
    // Creates matrix that transforms world coordinates to camera coordinates
    glm::mat4 getViewMatrix() {
        // GLM's lookAt function constructs view matrix from:
        // - Eye position: where camera is located
        // - Target point: where camera is looking (position + front)
        // - Up vector: defines camera's orientation
        return glm::lookAt(position, position + front, up);
    }

    // Orbit view matrix looks at a fixed target point
    glm::mat4 getOrbitViewMatrix() {
        // In orbit mode, camera always looks at the target
        return glm::lookAt(position, target, up);
    }

    // ========================================================================
    // FIRST-PERSON CAMERA CONTROLS (WASD MOVEMENT)
    // ========================================================================

    // Process keyboard input for camera movement
    // Uses camera's local coordinate system for natural movement
    void processKeyboard(int direction, float deltaTime) {
        // Calculate movement distance: speed × time = distance
        // This ensures frame-rate independent movement
        float velocity = movementSpeed * deltaTime;

        // Move along camera's front vector (forward/backward)
        if (direction == 0) // FORWARD (W key)
            position += front * velocity;    // Move in direction camera is facing
        if (direction == 1) // BACKWARD (S key)  
            position -= front * velocity;    // Move opposite to camera direction

        // Strafe along camera's right vector (left/right movement)
        if (direction == 2) // LEFT (A key)
            position -= right * velocity;    // Move left relative to camera
        if (direction == 3) // RIGHT (D key)
            position += right * velocity;    // Move right relative to camera

        // Vertical movement along camera's up vector
        if (direction == 4) // UP (Q key)
            position += up * velocity;       // Move up (fly/jump)
        if (direction == 5) // DOWN (E key)
            position -= up * velocity;       // Move down (crouch/descend)
    }

    // ========================================================================
    // MOUSE-BASED CAMERA ROTATION
    // ========================================================================

    // Process mouse movement to control camera orientation
    void processMouseMovement(float xoffset, float yoffset) {
        // Scale raw mouse input by sensitivity
        xoffset *= mouseSensitivity;
        yoffset *= mouseSensitivity;

        // Accumulate rotation angles
        // Mouse X movement controls yaw (horizontal rotation)
        yaw += xoffset;
        // Mouse Y movement controls pitch (vertical rotation)  
        pitch += yoffset;

        // Constrain pitch to prevent camera flipping (gimbal lock prevention)
        // Limit to slightly less than 90° to avoid mathematical singularities
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // Recalculate camera direction vectors from new angles
        updateCameraVectors();
    }

    // Process mouse scroll for zoom (FOV adjustment)
    void processMouseScroll(float yoffset) {
        fov -= yoffset;  // Scroll up = zoom in (smaller FOV)

        // Clamp FOV to reasonable range
        if (fov < 1.0f) fov = 1.0f;     // Minimum zoom
        if (fov > 45.0f) fov = 45.0f;   // Maximum zoom
    }

    // ========================================================================
    // ORBIT CAMERA UPDATE
    // ========================================================================

    // Update camera position for orbit mode
    void updateOrbitCamera() {
        // Convert spherical coordinates to Cartesian coordinates
        // This positions camera on a sphere around the target
        float x = target.x + orbitDistance * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        float y = target.y + orbitDistance * sin(glm::radians(pitch));
        float z = target.z + orbitDistance * sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        position = glm::vec3(x, y, z);

        // Always point toward the target
        front = glm::normalize(target - position);
        updateCameraVectors();
    }

private:
    // ========================================================================
    // COORDINATE SYSTEM CALCULATION
    // ========================================================================

    // Convert Euler angles (yaw, pitch) to direction vectors
    // This is the core mathematical transformation for camera orientation
    void updateCameraVectors() {
        // Convert spherical coordinates to Cartesian direction vector
        glm::vec3 newFront;

        // Calculate front vector from yaw and pitch
        // Uses spherical coordinate conversion:
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        // Calculate right and up vectors using cross products
        // Right vector: perpendicular to both front and world up
        right = glm::normalize(glm::cross(front, worldUp));

        // Up vector: perpendicular to both right and front
        // This creates an orthonormal basis for the camera coordinate system
        up = glm::normalize(glm::cross(right, front));
    }
};