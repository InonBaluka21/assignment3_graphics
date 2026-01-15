#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 startPos)
    : m_Width(width), m_Height(height), m_Position(startPos)
{
    m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_Fov = 45.0f;
    m_Near = 0.1f;
    m_Far = 100.0f;
    UpdateCameraVectors();
}

void Camera::SetPerspective(float fovDegrees, float nearPlane, float farPlane)
{
    m_Fov = fovDegrees;
    m_Near = nearPlane;
    m_Far = farPlane;
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    float aspect = (float)m_Width / (float)m_Height;
    return glm::perspective(glm::radians(m_Fov), aspect, m_Near, m_Far);
}

void Camera::Pan(float xOffset, float yOffset)
{
    float speed = 0.05f;
    m_Position -= m_Right * xOffset * speed;
    m_Position -= m_Up * yOffset * speed;
}

void Camera::Zoom(float yOffset)
{
    float speed = 1.0f;
    m_Position += m_Front * yOffset * speed;
}

void Camera::UpdateCameraVectors()
{
    // Fixed camera orientation looking at -Z
    m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
}