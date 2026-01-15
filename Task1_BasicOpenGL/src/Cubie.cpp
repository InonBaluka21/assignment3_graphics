#include "Cubie.h"

static void Cycle4(StickerColor& a, StickerColor& b, StickerColor& c, StickerColor& d, bool clockwise)
{
    if (clockwise) 
    {
        StickerColor tmp = d; d = c; c = b; b = a; a = tmp;
    }
    else           
    { 
        StickerColor tmp = a; a = b; b = c; c = d; d = tmp; 
    }
}

glm::mat4 Cubie::BuildModel(const glm::vec3& slotWorldPos, const glm::mat4& wallAnimRotation, float uniformScale) const
{
    glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(uniformScale));

    //glm::mat4 trans = glm::translate(glm::mat4(1.0f), slotWorldPos);
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), slotWorldPos + translationOffset);

    return wallAnimRotation * trans * localRotation * scl;
}

void Cubie::ApplyLocalRotation(const glm::mat4& rot)
{
    localRotation = rot * localRotation;
}

void Cubie::RotateStickersAboutX(bool clockwise)
{
    auto& posY = stickers[(int)Face::PosY];
    auto& posZ = stickers[(int)Face::PosZ];
    auto& negY = stickers[(int)Face::NegY];
    auto& negZ = stickers[(int)Face::NegZ];
    Cycle4(posY, posZ, negY, negZ, clockwise);
}

void Cubie::RotateStickersAboutY(bool clockwise)
{
    auto& posZ = stickers[(int)Face::PosZ];
    auto& posX = stickers[(int)Face::PosX];
    auto& negZ = stickers[(int)Face::NegZ];
    auto& negX = stickers[(int)Face::NegX];
    Cycle4(posZ, posX, negZ, negX, clockwise);
}

void Cubie::RotateStickersAboutZ(bool clockwise)
{
    auto& posX = stickers[(int)Face::PosX];
    auto& posY = stickers[(int)Face::PosY];
    auto& negX = stickers[(int)Face::NegX];
    auto& negY = stickers[(int)Face::NegY];
    Cycle4(posX, posY, negX, negY, clockwise);
}