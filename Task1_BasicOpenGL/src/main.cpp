#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Debugger.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"

#include "Cubie.h"
#include "CubeMesh.h"

#include <iostream>

const unsigned int width = 800;
const unsigned int height = 800;
const float nearPlane = 0.1f;
const float farPlane  = 100.0f;
static glm::vec4 DarkPlastic()
{
    return glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Static Cubie (CubeMesh)", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);

    // Use the same style your engine already uses (it works in your project)
    gladLoadGL();

    glfwSwapInterval(1);
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // GPU resources (created once)
    CubeMesh cubeMesh;
    Texture texture("res/textures/white.png");
    Shader shader("res/shaders/basic.shader");

    // Camera (only for View matrix right now)
    Camera camera(width, height);
    camera.SetOrthographic(nearPlane, farPlane);
    camera.EnableInputs(window);

    // Perspective projection (3D)
    glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                      (float)width / (float)height,
                                      nearPlane, farPlane);

    // One cubie (logic object)
    Cubie cubie;
    cubie.id = 0;

    // Corner example: +X +Y +Z are colored, rest are internal/none.
    cubie.stickers[(int)Face::PosX] = StickerColor::Red;
    cubie.stickers[(int)Face::PosY] = StickerColor::White;
    cubie.stickers[(int)Face::PosZ] = StickerColor::Blue;

    cubie.localRotation = glm::mat4(1.0f);


    glm::vec3 slotWorldPos(0.0f, 0.0f, -3.0f);

    while (!glfwWindowShouldClose(window))
    {
        GLCall(glClearColor(0,0,0,1));
        GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        glm::mat4 view = camera.GetViewMatrix();

        // Continuous rotation (demo)
        float t = (float)glfwGetTime();
        glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f), glm::vec3(1, 0, 0));

        // Continuous yaw (spin)
        glm::mat4 yaw   = glm::rotate(glm::mat4(1.0f), t, glm::vec3(0, 1, 0));
        glm::mat4 wallAnimRotation = yaw * pitch;// * glm::rotate(glm::mat4(1.0f), 0.5f * t, glm::vec3(1, 0, 0));

        glm::mat4 model = cubie.BuildModel(slotWorldPos, wallAnimRotation, 1.0f);
        glm::mat4 mvp = proj * view * model;

        shader.Bind();
        texture.Bind(0);
        shader.SetUniform1i("u_Texture", 0);
        shader.SetUniformMat4f("u_MVP", mvp);

        // Bind mesh once, draw 6 faces with different colors.
        cubeMesh.Bind();

        // Draw-group -> Face enum mapping (based on cubeIndices order)
        // group 0: +Z, group 1: -Z, group 2: +X, group 3: -X, group 4: +Y, group 5: -Y
        Face groupToFace[6] = {
            Face::PosZ, Face::NegZ,
            Face::PosX, Face::NegX,
            Face::PosY, Face::NegY
        };

        for (int group = 0; group < 6; ++group)
        {
            Face f = groupToFace[group];
            StickerColor sc = cubie.stickers[(int)f];

            glm::vec4 faceColor = (sc == StickerColor::None) ? DarkPlastic()
                                                            : StickerToVec4(sc);

            shader.SetUniform4f("u_Color", faceColor);

            // each face is 6 indices; offset is group*6*sizeof(uint)
            const void* offset = (const void*)(group * 6 * sizeof(unsigned int));
            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, offset));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwTerminate();
    return 0;
}
