#pragma once

#include "Cubie.h"
#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Texture.h"
#include "CubeMesh.h"

class RubiksCube
{
public:
    RubiksCube(int size = 3);
    ~RubiksCube();

    void Init();
    
    // Standard Draw
    void Draw(const glm::mat4& viewProj, const glm::mat4& globalModel, 
              bool isAnimating = false, glm::vec3 animAxis = glm::vec3(0), float animDeg = 0.0f, 
              int layerIndex = -1, int highlightedId = -1);
              
    // Picking Draw (Colors = IDs)
    void DrawPicking(const glm::mat4& viewProj, const glm::mat4& globalModel);

    void FinishTurn(glm::vec3 axis, float deg, int layerIndex = -1);
    
    // NEW: Function to manipulate a single picked cube
    void UpdateCubieDesync(int id, const glm::mat4& deltaTransform);

    int GetSize() const { return m_Size; }

private:
    void SetupStickers(Cubie& cubie, int x, int y, int z);
    glm::vec3 GetInitialPosition(int x, int y, int z) const;

    int m_Size;
    std::vector<Cubie> m_Cubies;
    
    CubeMesh* m_Mesh;
    Shader* m_Shader;
    Texture* m_Texture;
};