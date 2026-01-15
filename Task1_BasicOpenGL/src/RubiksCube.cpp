#include "RubiksCube.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

// Distance between cubies
static const float SPACING = 2.1f; 

RubiksCube::RubiksCube(int size)
    : m_Size(size), m_Mesh(nullptr), m_Shader(nullptr), m_Texture(nullptr)
{
    if (m_Size < 1) m_Size = 1;

    m_Mesh = new CubeMesh();
    m_Shader = new Shader("res/shaders/basic.shader");
    m_Texture = new Texture("res/textures/white.png");

    Init();
}

RubiksCube::~RubiksCube()
{
    delete m_Mesh;
    delete m_Shader;
    delete m_Texture;
}

void RubiksCube::Init()
{
    m_Cubies.clear();
    int idCounter = 0;

    for (int x = 0; x < m_Size; x++)
    {
        for (int y = 0; y < m_Size; y++)
        {
            for (int z = 0; z < m_Size; z++)
            {
                Cubie newCubie;
                newCubie.id = idCounter++;
                newCubie.currentGridPos = glm::ivec3(x, y, z); 
                newCubie.localRotation = glm::mat4(1.0f);

                SetupStickers(newCubie, x, y, z);

                m_Cubies.push_back(newCubie);
            }
        }
    }
}

void RubiksCube::SetupStickers(Cubie& cubie, int x, int y, int z)
{
    for (int i = 0; i < 6; i++) 
        cubie.stickers[i] = StickerColor::None;

    if (x == m_Size - 1)  cubie.stickers[(int)Face::PosX] = StickerColor::Red;
    if (x == 0)           cubie.stickers[(int)Face::NegX] = StickerColor::Orange;
    if (y == m_Size - 1)  cubie.stickers[(int)Face::PosY] = StickerColor::White;
    if (y == 0)           cubie.stickers[(int)Face::NegY] = StickerColor::Yellow;
    if (z == m_Size - 1)  cubie.stickers[(int)Face::PosZ] = StickerColor::Blue;
    if (z == 0)           cubie.stickers[(int)Face::NegZ] = StickerColor::Green;
}

glm::vec3 RubiksCube::GetInitialPosition(int x, int y, int z) const
{
    float offset = (m_Size - 1) / 2.0f;
    return glm::vec3((x - offset) * SPACING, (y - offset) * SPACING, (z - offset) * SPACING);
}

void RubiksCube::Draw(const glm::mat4& viewProj, const glm::mat4& globalModel, 
                      bool isAnimating, glm::vec3 animAxis, float animDeg, 
                      int layerIndex, int highlightedId)
{
    m_Shader->Bind();
    m_Texture->Bind(0);
    m_Shader->SetUniform1i("u_Texture", 0);
    m_Mesh->Bind();

    for (const auto& cubie : m_Cubies)
    {
        int x = cubie.currentGridPos.x;
        int y = cubie.currentGridPos.y;
        int z = cubie.currentGridPos.z;

        glm::vec3 currentPos = GetInitialPosition(x, y, z);
        glm::mat4 animRot = glm::mat4(1.0f);

        if (isAnimating)
        {
            bool shouldRotate = false;

            if (layerIndex == -1)
            {
                if      (animAxis.x > 0.5f && x == m_Size - 1) shouldRotate = true; 
                else if (animAxis.x < -0.5f && x == 0)           shouldRotate = true; 
                else if (animAxis.y > 0.5f && y == m_Size - 1) shouldRotate = true; 
                else if (animAxis.y < -0.5f && y == 0)           shouldRotate = true; 
                else if (animAxis.z > 0.5f && z == m_Size - 1) shouldRotate = true; 
                else if (animAxis.z < -0.5f && z == 0)           shouldRotate = true; 
            }
            else
            {
                if      (std::abs(animAxis.x) > 0.9f && x == layerIndex) shouldRotate = true;
                else if (std::abs(animAxis.y) > 0.9f && y == layerIndex) shouldRotate = true;
                else if (std::abs(animAxis.z) > 0.9f && z == layerIndex) shouldRotate = true;
            }

            if (shouldRotate)
            {
                animRot = glm::rotate(glm::mat4(1.0f), glm::radians(animDeg), animAxis);
            }
        }

        // --- Corrected: No Scaling, just draw the model ---
        // If you want to highlight, you could change the u_Color slightly, 
        // but the requirement is Translation/Rotation, so we leave scale at 1.0f.
        glm::mat4 model = globalModel * cubie.BuildModel(currentPos, animRot, 1.0f);
        glm::mat4 mvp = viewProj * model;

        m_Shader->SetUniformMat4f("u_MVP", mvp);

        Face groupToFace[6] = {
            Face::PosZ, Face::NegZ,
            Face::PosX, Face::NegX,
            Face::PosY, Face::NegY
        };

        for (int group = 0; group < 6; ++group)
        {
            Face f = groupToFace[group];
            StickerColor sc = cubie.stickers[(int)f];

            glm::vec4 colorVec;
            if (sc == StickerColor::None)
                colorVec = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f); 
            else
                colorVec = StickerToVec4(sc);

            m_Shader->SetUniform4f("u_Color", colorVec);

            const void* offset = (const void*)(group * 6 * sizeof(unsigned int));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, offset);
        }
    }
}

void RubiksCube::DrawPicking(const glm::mat4& viewProj, const glm::mat4& globalModel)
{
    m_Shader->Bind();
    m_Shader->SetUniform1i("u_PickingMode", 1); 
    m_Mesh->Bind();

    for (const auto& cubie : m_Cubies)
    {
        int x = cubie.currentGridPos.x;
        int y = cubie.currentGridPos.y;
        int z = cubie.currentGridPos.z;

        glm::vec3 currentPos = GetInitialPosition(x, y, z);
        
        // For picking, we assume no active animation keyframe
        glm::mat4 model = globalModel * cubie.BuildModel(currentPos, glm::mat4(1.0f), 1.0f);
        glm::mat4 mvp = viewProj * model;

        m_Shader->SetUniformMat4f("u_MVP", mvp);

        // Encode ID
        float r = (float)cubie.id / 255.0f;
        glm::vec4 idColor = glm::vec4(r, 0.0f, 0.0f, 1.0f);
        m_Shader->SetUniform4f("u_Color", idColor);

        glDrawElements(GL_TRIANGLES, m_Mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
    }
    m_Shader->SetUniform1i("u_PickingMode", 0);
}

void RubiksCube::UpdateCubieDesync(int id, const glm::mat4& deltaTransform)
{
    // Apply a transformation to a specific cubie (for the bonus requirement)
    for (auto& cubie : m_Cubies)
    {
        if (cubie.id == id)
        {
            // Apply the transformation on top of its existing local rotation
            cubie.localRotation = deltaTransform * cubie.localRotation;
            break;
        }
    }
}

void RubiksCube::FinishTurn(glm::vec3 axis, float deg, int layerIndex)
{
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(deg), axis);
    float center = (m_Size - 1) / 2.0f;

    for (auto& cubie : m_Cubies)
    {
        bool shouldRotate = false;
        int x = cubie.currentGridPos.x;
        int y = cubie.currentGridPos.y;
        int z = cubie.currentGridPos.z;

        if (layerIndex == -1)
        {
            if      (axis.x > 0.5f && x == m_Size - 1) shouldRotate = true;
            else if (axis.x < -0.5f && x == 0)           shouldRotate = true;
            else if (axis.y > 0.5f && y == m_Size - 1) shouldRotate = true;
            else if (axis.y < -0.5f && y == 0)           shouldRotate = true;
            else if (axis.z > 0.5f && z == m_Size - 1) shouldRotate = true;
            else if (axis.z < -0.5f && z == 0)           shouldRotate = true;
        }
        else
        {
            if      (std::abs(axis.x) > 0.9f && x == layerIndex) shouldRotate = true;
            else if (std::abs(axis.y) > 0.9f && y == layerIndex) shouldRotate = true;
            else if (std::abs(axis.z) > 0.9f && z == layerIndex) shouldRotate = true;
        }

        if (shouldRotate)
        {
            glm::vec4 p(x - center, y - center, z - center, 1.0f);
            glm::vec4 pNew = rot * p;

            cubie.currentGridPos.x = (int)std::round(pNew.x + center);
            cubie.currentGridPos.y = (int)std::round(pNew.y + center);
            cubie.currentGridPos.z = (int)std::round(pNew.z + center);

            cubie.localRotation = rot * cubie.localRotation;
        }
    }
}