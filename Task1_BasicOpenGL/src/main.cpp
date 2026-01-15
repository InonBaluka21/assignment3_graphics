#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "RubiksCube.h" 

#include <iostream>
#include <algorithm> // For std::min, std::max

// Window settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// Application State
struct AppState {
    Camera* camera = nullptr;
    
    // Mouse State
    bool firstMouse = true;
    double lastX = SCR_WIDTH / 2.0;
    double lastY = SCR_HEIGHT / 2.0;
    bool leftMousePressed = false;
    bool rightMousePressed = false;

    // Cube Logic
    RubiksCube* cube = nullptr;
    glm::mat4 globalCubeRotation = glm::mat4(1.0f);

    // Bonus: Selection Logic (Center of Rotation)
    // Default center for size 3 is index 1. For size 5 it is index 2.
    int selectedLayerX = 1;
    int selectedLayerY = 1;
    int selectedLayerZ = 1;

    // Turn Logic
    bool isTurning = false;
    glm::vec3 turnAxis = glm::vec3(0);
    float turnTargetDeg = 90.0f;
    float turnCurrentDeg = 0.0f;
    float turnSpeed = 180.0f; // degrees per second
    int turnDir = 1; // 1 or -1
    float stepDeg = 90.0f;
    
    // Which index is currently active
    int currentActiveLayerIndex = -1; 

    bool isPickingMode = false;
    int pickedCubieId = -1;
    float pickedDepth = 0.0f;
};

// Input Callbacks declarations
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
glm::vec3 GetWorldPos(double mouseX, double mouseY, float depth, Camera* cam) 
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // OpenGL Y is inverted (0 at bottom), Mouse Y is 0 at top
    glm::vec3 winCoord = glm::vec3(mouseX, viewport[3] - mouseY, depth);
    
    glm::mat4 view = cam->GetViewMatrix();
    glm::mat4 projection = cam->GetProjectionMatrix();

    return glm::unProject(winCoord, view, projection, glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
}

int main()
{
    // Initialize GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rubik's Cube Assignment", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    // --- SCOPE START: Objects must be destroyed before glfwTerminate ---
    {
        glEnable(GL_DEPTH_TEST);

        // Initialize State
        AppState state;
        Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 15.0f));
        
        // Change cube size here (Bonus)
        int cubeSize = 3; 
        RubiksCube rubiksCube(cubeSize);
        
        state.camera = &camera;
        state.cube = &rubiksCube;
        
        // Initialize selection to center
        state.selectedLayerX = cubeSize / 2;
        state.selectedLayerY = cubeSize / 2;
        state.selectedLayerZ = cubeSize / 2;

        glfwSetWindowUserPointer(window, &state);

        // Callbacks
        glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
        glfwSetCursorPosCallback(window, MouseCallback);
        glfwSetScrollCallback(window, ScrollCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetKeyCallback(window, KeyCallback);

        // Timing
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        std::cout << "Controls:\n";
        std::cout << "Arrows Left/Right: Change X Layer Selection\n";
        std::cout << "Arrows Up/Down: Change Y Layer Selection\n";
        std::cout << "I / O: Change Z Layer Selection\n";
        std::cout << "R/L/U/D/F/B: Rotate the SELECTED layer on that axis\n";
        std::cout << "Space: Reverse direction\n";

        while (!glfwWindowShouldClose(window))
        {
            float currentFrame = (float)glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Render
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Update Animation
            if (state.isTurning)
            {
                float step = state.turnSpeed * deltaTime;
                state.turnCurrentDeg += step;
                if (state.turnCurrentDeg >= std::abs(state.turnTargetDeg))
                {
                    state.turnCurrentDeg = std::abs(state.turnTargetDeg);
                    state.isTurning = false;
                    // Pass the active index to FinishTurn
                    state.cube->FinishTurn(state.turnAxis, state.turnTargetDeg, state.currentActiveLayerIndex);
                }
            }

            // Matrices
            glm::mat4 view = state.camera->GetViewMatrix();
            glm::mat4 projection = state.camera->GetProjectionMatrix();
            glm::mat4 viewProj = projection * view;
            glm::mat4 model = state.globalCubeRotation; 

            // Animation Params
            bool isAnim = state.isTurning;
            float currentAnimAngle = (state.turnTargetDeg < 0) ? -state.turnCurrentDeg : state.turnCurrentDeg;

            // Pass the active index to Draw
            state.cube->Draw(viewProj, model, isAnim, state.turnAxis, currentAnimAngle, state.currentActiveLayerIndex);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    } // --- SCOPE END: Destructors run here while OpenGL context is still valid ---

    glfwTerminate();
    return 0;
}

// Callbacks implementation...
// (Make sure to include the callback function implementations below or in a separate file as before)
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (s && s->camera) s->camera->UpdateSize(width, height);
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (!s) return;

    if (s->firstMouse)
    {
        s->lastX = xpos;
        s->lastY = ypos;
        s->firstMouse = false;
    }

    float xoffset = (float)(xpos - s->lastX);
    float yoffset = (float)(s->lastY - ypos); 

    s->lastX = xpos;
    s->lastY = ypos;

    if (s->rightMousePressed)
    {
        if (s->isPickingMode && s->pickedCubieId != -1)
        {
            glm::vec3 globalMousePos = GetWorldPos(xpos, ypos, s->pickedDepth, s->camera);

            glm::mat4 inverseModel = glm::inverse(s->globalCubeRotation);
            
            glm::vec4 localMousePos4 = inverseModel * glm::vec4(globalMousePos, 1.0f);
            glm::vec3 localMousePos = glm::vec3(localMousePos4);

            s->cube->SetCubiePosition(s->pickedCubieId, localMousePos);
        }
        else 
        {
            s->camera->Pan(-xoffset, -yoffset);
        }
    }

    if (s->leftMousePressed)
    {
        if (s->isPickingMode && s->pickedCubieId != -1)
        {
            float sensitivity = 0.05f; 
            glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), xoffset * sensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), -yoffset * sensitivity, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 deltaRot = rotX * rotY;
            s->cube->UpdateCubieDesync(s->pickedCubieId, deltaRot);
        }
        else if (!s->isPickingMode)
        {
            float sensitivity = 0.005f;
            glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), xoffset * sensitivity, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), -yoffset * sensitivity, glm::vec3(1.0f, 0.0f, 0.0f));
            s->globalCubeRotation = rotX * rotY * s->globalCubeRotation;
        }
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (s && s->camera)
    {
        s->camera->Zoom((float)yoffset);
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (!s) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS) 
        {
            s->leftMousePressed = true;

            // --- COLOR PICKING LOGIC (Only on Click) ---
            if (s->isPickingMode)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                // 1. Clear Screen for Picking Draw
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // 2. Draw Picking Scene
                glm::mat4 view = s->camera->GetViewMatrix();
                glm::mat4 projection = s->camera->GetProjectionMatrix();
                glm::mat4 viewProj = projection * view;
                
                s->cube->DrawPicking(viewProj, s->globalCubeRotation);

                // 3. Read Pixel
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                
                int winWidth, winHeight;
                glfwGetWindowSize(window, &winWidth, &winHeight);
                float xScale = (float)width / (float)winWidth;
                float yScale = (float)height / (float)winHeight;

                int pixelX = (int)(xpos * xScale);
                int pixelY = (int)((winHeight - ypos) * yScale); // Invert Y
                float depth;
                glReadPixels(pixelX, pixelY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
                s->pickedDepth = depth;

                unsigned char data[4];
                glReadPixels(pixelX, pixelY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

                // 4. Decode ID
                if (data[0] == 0 && data[1] == 0 && data[2] == 0) 
                {
                    s->pickedCubieId = -1;
                    std::cout << "Picked: None" << std::endl;
                }
                else 
                {
                    int id = (int)data[0]; 
                    s->pickedCubieId = id;
                    std::cout << "Picked Cubie ID: " << id << std::endl;
                }
            }
        }
        else if (action == GLFW_RELEASE) 
        {
            s->leftMousePressed = false;
        }
    }
    
    // Right Click logic
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS) s->rightMousePressed = true;
        else if (action == GLFW_RELEASE) s->rightMousePressed = false;
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    AppState* s = (AppState*)glfwGetWindowUserPointer(window);
    if (!s || action != GLFW_PRESS) return;

    // 1. General Settings
    if (key == GLFW_KEY_SPACE) { s->turnDir *= -1; std::cout << "Direction flipped\n"; return; }
    if (key == GLFW_KEY_Z) { s->stepDeg = std::max(90.0f, s->stepDeg / 2.0f); return; }
    if (key == GLFW_KEY_A) { s->stepDeg = std::min(180.0f, s->stepDeg * 2.0f); return; }

    int maxIndex = s->cube->GetSize() - 1;

    // 2. Layer Selection Logic (Bonus)
    // X Axis Selection
    if (key == GLFW_KEY_RIGHT) {
        s->selectedLayerX = std::min(maxIndex, s->selectedLayerX + 1);
        std::cout << "Selected X Layer: " << s->selectedLayerX << std::endl;
        return;
    }
    if (key == GLFW_KEY_LEFT) {
        s->selectedLayerX = std::max(0, s->selectedLayerX - 1);
        std::cout << "Selected X Layer: " << s->selectedLayerX << std::endl;
        return;
    }

    // Y Axis Selection
    if (key == GLFW_KEY_UP) {
        s->selectedLayerY = std::min(maxIndex, s->selectedLayerY + 1);
        std::cout << "Selected Y Layer: " << s->selectedLayerY << std::endl;
        return;
    }
    if (key == GLFW_KEY_DOWN) {
        s->selectedLayerY = std::max(0, s->selectedLayerY - 1);
        std::cout << "Selected Y Layer: " << s->selectedLayerY << std::endl;
        return;
    }

    // Z Axis Selection
    if (key == GLFW_KEY_I) { // In
        s->selectedLayerZ = std::max(0, s->selectedLayerZ - 1);
        std::cout << "Selected Z Layer: " << s->selectedLayerZ << std::endl;
        return;
    }
    if (key == GLFW_KEY_O) { // Out
        s->selectedLayerZ = std::min(maxIndex, s->selectedLayerZ + 1);
        std::cout << "Selected Z Layer: " << s->selectedLayerZ << std::endl;
        return;
    }
    if (key == GLFW_KEY_P) 
    { 
        s->isPickingMode = !s->isPickingMode; 
        std::cout << "Picking Mode: " << (s->isPickingMode ? "ON" : "OFF") << std::endl;
        return; 
    }

    // 3. Perform Rotation
    if (s->isTurning) return; 

    glm::vec3 axis(0.0f);
    bool validKey = true;
    int targetLayer = -1; 

    switch (key)
    {
        // X Axis Rotations
        case GLFW_KEY_R: 
            axis = glm::vec3(1, 0, 0); 
            targetLayer = s->selectedLayerX; 
            break;
        case GLFW_KEY_L: 
            axis = glm::vec3(-1, 0, 0); 
            targetLayer = s->selectedLayerX;
            break;

        // Y Axis Rotations
        case GLFW_KEY_U: 
            axis = glm::vec3(0, 1, 0); 
            targetLayer = s->selectedLayerY; 
            break;
        case GLFW_KEY_D: 
            axis = glm::vec3(0, -1, 0); 
            targetLayer = s->selectedLayerY;
            break;

        // Z Axis Rotations
        case GLFW_KEY_F: 
            axis = glm::vec3(0, 0, 1); 
            targetLayer = s->selectedLayerZ; 
            break;
        case GLFW_KEY_B: 
            axis = glm::vec3(0, 0, -1); 
            targetLayer = s->selectedLayerZ;
            break;

        default: validKey = false; break;
    }

    if (validKey)
    {
        s->isTurning = true;
        s->turnAxis = axis;
        s->turnTargetDeg = (float)s->turnDir * s->stepDeg;
        s->turnCurrentDeg = 0.0f;
        s->currentActiveLayerIndex = targetLayer;
    }
}