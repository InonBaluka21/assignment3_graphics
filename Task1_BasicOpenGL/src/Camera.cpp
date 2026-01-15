#include "Camera.h"

// Correct GLFW callback signatures: key uses (window, key, scancode, action, mods),
// mouse button uses (window, button, action, mods),
// cursor pos uses (window, xpos, ypos),
// scroll uses (window, xoffset, yoffset). [web:16][web:27]
static void KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
            case GLFW_KEY_UP:    std::cout << "UP Pressed\n"; break;
            case GLFW_KEY_DOWN:  std::cout << "DOWN Pressed\n"; break;
            case GLFW_KEY_LEFT:  std::cout << "LEFT Pressed\n"; break;
            case GLFW_KEY_RIGHT: std::cout << "RIGHT Pressed\n"; break;
            default: break;
        }
    }
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        std::cout << "MOUSE LEFT Click\n";
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        std::cout << "MOUSE RIGHT Click\n";
}

static void CursorPosCallback(GLFWwindow* window, double currMouseX, double currMouseY)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    // חישוב הדלתא (כמה העכבר זז מהפריים הקודם)
    double deltaX = currMouseX - camera->m_OldMouseX;
    double deltaY = camera->m_OldMouseY - currMouseY; // הפוך ב-Y כי בצגים למעלה זה 0

    camera->m_OldMouseX = currMouseX;
    camera->m_OldMouseY = currMouseY;

    // בדיקה: האם כפתור ימני לחוץ? רק אז נזיז את המצלמה
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        float sensitivity = 0.3f; // רגישות העכבר
        camera->m_Yaw   += deltaX * sensitivity;
        camera->m_Pitch += deltaY * sensitivity;

        // הגבלה כדי שהמצלמה לא תתהפך (Gimbal Lock)
        if (camera->m_Pitch > 89.0f) camera->m_Pitch = 89.0f;
        if (camera->m_Pitch < -89.0f) camera->m_Pitch = -89.0f;

        // עדכון המיקום
        camera->RecalculatePosition();
    }
}

static void ScrollCallback(GLFWwindow* window, double scrollOffsetX, double scrollOffsetY)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    std::cout << "SCROLL Motion\n";
}

void Camera::UpdateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
}

void Camera::SetPerspective(float fovDegrees, float nearPlane, float farPlane)
{
    m_Near = nearPlane;
    m_Far  = farPlane;

    float aspect = (float)m_Width / (float)m_Height;
    m_Projection = glm::perspective(glm::radians(fovDegrees), aspect, m_Near, m_Far);
    UpdateView();
}

void Camera::EnableInputs(GLFWwindow* window)
{
    glfwSetWindowUserPointer(window, this);

    // Register callbacks without any casts (signatures already match). [web:16][web:27]
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}

void Camera::LookAt(const glm::vec3& target)
{
    m_Orientation = glm::normalize(target - m_Position);
    UpdateView();
}

void Camera::RecalculatePosition()
{
    // המרת קואורדינטות כדוריות (זוויות) לקואורדינטות קרטזיות (XYZ)
    float x = m_Radius * cos(glm::radians(m_Pitch)) * cos(glm::radians(m_Yaw));
    float y = m_Radius * sin(glm::radians(m_Pitch));
    float z = m_Radius * cos(glm::radians(m_Pitch)) * sin(glm::radians(m_Yaw));

    m_Position = glm::vec3(x, y, z);
    
    // חשוב: תמיד להסתכל למרכז (0,0,0)
    LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
}