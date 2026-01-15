#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Fov;
    float m_Near;
    float m_Far;
    int m_Width;
    int m_Height;

public:
    Camera(int width, int height, glm::vec3 startPos);

    void SetPerspective(float fovDegrees, float nearPlane, float farPlane);
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // Camera Transformations
    void Pan(float xOffset, float yOffset);
    void Zoom(float yOffset);
    
    // Getters/Setters
    glm::vec3 GetPosition() const { return m_Position; }
    void UpdateSize(int width, int height) { m_Width = width; m_Height = height; }

private:
    void UpdateCameraVectors();
};