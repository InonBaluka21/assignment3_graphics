#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Cubie.h"
#include "CubeMesh.h"
#include "Shader.h"
#include "Texture.h"

class RubiksCube
{
private:
    std::vector<Cubie> m_Cubies;
    CubeMesh* m_Mesh;
    Shader* m_Shader;
    Texture* m_Texture;

public:
    RubiksCube();
    ~RubiksCube();

    void Init();
    void Draw(const glm::mat4& viewProj, const glm::mat4& globalModel);
    void Draw(const glm::mat4& viewProj, const glm::mat4& globalModel, bool isAnimating, glm::vec3 animAxis, float animDeg);
    void FinishTurn(glm::vec3 axis, float deg);

private:
    glm::vec3 GetInitialPosition(int x, int y, int z) const;
    void SetupStickers(Cubie& cubie, int x, int y, int z);
};