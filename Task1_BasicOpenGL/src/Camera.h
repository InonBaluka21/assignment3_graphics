#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

class Camera
{
private:
    glm::mat4 m_View       = glm::mat4(1.0f);
    glm::mat4 m_Projection = glm::mat4(1.0f);

    glm::vec3 m_Position    = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 m_Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_Up          = glm::vec3(0.0f, 1.0f, 0.0f);

    float m_Near = 0.1f;
    float m_Far  = 100.0f;

    int m_Width  = 0;
    int m_Height = 0;
    
public:
    float m_Yaw = -90.0f;   // זווית אופקית (מתחילים ממול)
    float m_Pitch = 0.0f;   // זווית אנכית
    float m_Radius = 15.0f; // מרחק מהמרכז

    double m_OldMouseX = 0.0, m_OldMouseY = 0.0;
    double m_NewMouseX = 0.0, m_NewMouseY = 0.0;
    
    Camera(int width, int height) : m_Width(width), m_Height(height) {}
    
    void SetPerspective(float fovDegrees, float nearPlane, float farPlane);
    void UpdateView();
    void EnableInputs(GLFWwindow* window);
    void LookAt(const glm::vec3& target);
    
    glm::mat4 GetViewMatrix() const { return m_View; }
    glm::mat4 GetProjectionMatrix() const { return m_Projection; }
    
    glm::vec3 GetPosition() const { return m_Position; }
    void SetPosition(const glm::vec3& p) { 
        m_Position = p;
        UpdateView(); 
    }
    void RecalculatePosition();
};
