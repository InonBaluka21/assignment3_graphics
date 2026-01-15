#pragma once
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class Face : int { PosX=0, NegX=1, PosY=2, NegY=3, PosZ=4, NegZ=5 };

enum class StickerColor : int {
    None = -1,
    White, Yellow, Red, Orange, Blue, Green
};

inline glm::vec4 StickerToVec4(StickerColor c)
{
    switch (c)
    {
        case StickerColor::White:  return {1,1,1,1};
        case StickerColor::Yellow: return {1,1,0,1};
        case StickerColor::Red:    return {1,0,0,1};
        case StickerColor::Orange: return {1,0.5f,0,1};
        case StickerColor::Blue:   return {0,0,1,1};
        case StickerColor::Green:  return {0,1,0,1};
        default:                   return {0,0,0,0};
    }
}

struct Cubie
{
    int id = -1;
    glm::ivec3 currentGridPos;
    // Stickers by face direction; internal faces are StickerColor::None.
    std::array<StickerColor, 6> stickers = {
        StickerColor::None, StickerColor::None,
        StickerColor::None, StickerColor::None,
        StickerColor::None, StickerColor::None
    };

    // Cubie's own local orientation (stickers rotate with this).
    glm::mat4 localRotation = glm::mat4(1.0f);

    // Build model matrix from: slot world pos + local orientation + optional animation rotation + scale.
    glm::mat4 BuildModel(const glm::vec3& slotWorldPos,
                         const glm::mat4& wallAnimRotation,
                         float uniformScale) const;

    // After a wall turn is finalized, update cubie's local orientation (and optionally stickers).
    void ApplyLocalRotation(const glm::mat4& rot);

    // Optional helper: rotate sticker assignment when the cubie turns 90° about axis.
    // (If you keep "stickers drawn by face direction", you'll want this.)
    void RotateStickersAboutX(bool clockwise);
    void RotateStickersAboutY(bool clockwise);
    void RotateStickersAboutZ(bool clockwise);
};