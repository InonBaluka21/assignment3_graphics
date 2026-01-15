#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class StickerColor
{
    White, Yellow, Green, Blue, Red, Orange, None
};

enum class Face
{
    PosY = 0, // Top
    NegY = 1, // Bottom
    NegZ = 2, // Back
    PosZ = 3, // Front
    PosX = 4, // Right
    NegX = 5  // Left
};

static glm::vec4 StickerToVec4(StickerColor color)
{
    switch (color)
    {
    case StickerColor::White:  return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    case StickerColor::Yellow: return glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    case StickerColor::Green:  return glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    case StickerColor::Blue:   return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    case StickerColor::Red:    return glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    case StickerColor::Orange: return glm::vec4(1.0f, 0.64f, 0.0f, 1.0f);
    case StickerColor::None:   return glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    }
    return glm::vec4(0.0f);
}

struct Cubie
{
    int id;
    glm::ivec3 currentGridPos;
    glm::mat4 localRotation;
    StickerColor stickers[6];
    glm::vec3 translationOffset = glm::vec3(0.0f);

    Cubie() : id(0), currentGridPos(0), localRotation(1.0f) {}

    glm::mat4 BuildModel(const glm::vec3& slotWorldPos, const glm::mat4& wallAnimRotation, float uniformScale) const;

    void ApplyLocalRotation(const glm::mat4& rot);
    void RotateStickersAboutX(bool clockwise);
    void RotateStickersAboutY(bool clockwise);
    void RotateStickersAboutZ(bool clockwise);
};