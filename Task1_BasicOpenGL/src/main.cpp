#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Debugger.h"
#include "Camera.h"
#include "RubiksCube.h" 

#include <iostream>

const unsigned int width = 800;
const unsigned int height = 800;
const float nearPlane = 0.1f;
const float farPlane  = 100.0f;

struct TurnState {
    bool active = false;
    glm::vec3 axis = glm::vec3(0,1,0);
    float targetDeg = 90.0f;   
    float currentDeg = 0.0f;   
    float speedDegPerSec = 180.0f;
};

struct AppState {
    TurnState turn;
    int dir = +1;             
    float stepDeg = 90.0f;    
    Camera* camera = nullptr; // חיבור למצלמה
};

static glm::vec3 KeyToAxis(int key)
{
    switch (key)
    {
        case GLFW_KEY_R: return glm::vec3( 1,0,0);
        case GLFW_KEY_L: return glm::vec3(-1,0,0);
        case GLFW_KEY_U: return glm::vec3( 0,1,0);
        case GLFW_KEY_D: return glm::vec3( 0,-1,0);
        case GLFW_KEY_F: return glm::vec3( 0,0,1);
        case GLFW_KEY_B: return glm::vec3( 0,0,-1);
        default:         return glm::vec3(0,0,0);
    }
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (!s) return;

    if (key == GLFW_KEY_SPACE) { s->dir *= -1; return; }
    if (key == GLFW_KEY_Z)     { s->stepDeg = std::max(90.0f, s->stepDeg / 2.0f); return; }
    if (key == GLFW_KEY_A)     { s->stepDeg = std::min(180.0f, s->stepDeg * 2.0f); return; }

    if (s->turn.active) return;

    glm::vec3 axis = KeyToAxis(key);
    if (axis == glm::vec3(0,0,0)) return;

    s->turn.active = true;
    s->turn.axis = glm::normalize(axis);
    s->turn.targetDeg = (float)s->dir * s->stepDeg;
    s->turn.currentDeg = 0.0f;
}

static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // 1. בדיקה שהחיבור ל-State עובד
    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (!s) {
        std::cout << "ERROR: State is NULL!" << std::endl;
        return;
    }
    if (!s->camera) {
        std::cout << "ERROR: Camera is NULL!" << std::endl;
        return;
    }

    Camera* cam = s->camera;

    // חישוב דלתא
    double deltaX = xpos - cam->m_OldMouseX;
    double deltaY = cam->m_OldMouseY - ypos; 

    // עדכון מיידי
    cam->m_OldMouseX = xpos;
    cam->m_OldMouseY = ypos;
    
    // רגישות גבוהה לבדיקה
    float sensitivity = 0.5f; 
    cam->m_Yaw   += deltaX * sensitivity;
    cam->m_Pitch += -deltaY * sensitivity;

    if (cam->m_Yaw > 360.0f) cam->m_Yaw -= 360.0f;
    if (cam->m_Yaw < 0.0f)   cam->m_Yaw += 360.0f;
    // ------------------------

    // הגבלת Pitch (כבר קיים אצלך)
    if (cam->m_Pitch > 89.0f) cam->m_Pitch = 89.0f;
    if (cam->m_Pitch < -89.0f) cam->m_Pitch = -89.0f;
    
    cam->RecalculatePosition();
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Rubiks Cube (Full)", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    AppState state;
    glfwSetWindowUserPointer(window, &state);
    
    // רישום Callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback); // חשוב!

    RubiksCube rubiksCube;

    // אתחול מצלמה
    Camera camera(width, height);
    state.camera = &camera; // חיבור המצלמה ל-state
    
    // אתחול המיקום ההתחלתי:
    // שינוי: הסרנו את SetPosition הידני שהיה כאן ודרס את החישוב
    glfwGetCursorPos(window, &camera.m_OldMouseX, &camera.m_OldMouseY);
    camera.SetPerspective(70.0f, nearPlane, farPlane);
    camera.RecalculatePosition(); // מציב את המצלמה בנקודת ההתחלה לפי רדיוס וזווית

    glm::mat4 globalRotation = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window))
    {
        GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1)); 
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // שליפת המטריצות המעודכנות מהמצלמה
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = camera.GetProjectionMatrix();
        glm::mat4 vp = proj * view;

        // לוגיקת אנימציה
        static double lastT = glfwGetTime();
        double nowT = glfwGetTime();
        float dt = (float)(nowT - lastT);
        lastT = nowT;

        if (state.turn.active)
        {
            float goal = std::abs(state.turn.targetDeg);
            state.turn.currentDeg += state.turn.speedDegPerSec * dt;
            
            if (state.turn.currentDeg >= goal)
            {
                state.turn.currentDeg = goal;
                state.turn.active = false;
                rubiksCube.FinishTurn(state.turn.axis, state.turn.targetDeg);
            }
        }
        
        bool isAnimating = state.turn.active;
        glm::vec3 animAxis = state.turn.axis;
        float animDeg = 0.0f;

        if (isAnimating) {
            float signedDeg = (state.turn.targetDeg < 0.0f) ? -state.turn.currentDeg : state.turn.currentDeg;
            animDeg = signedDeg;
        }

        glm::mat4 finalGlobalModel = globalRotation;
        rubiksCube.Draw(vp, finalGlobalModel, isAnimating, animAxis, animDeg);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}