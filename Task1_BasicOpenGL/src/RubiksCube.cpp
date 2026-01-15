#include "RubiksCube.h"
#include <iostream>

// Distance between cubies to make them look separate
static const float SPACING = 2.1f; 

RubiksCube::RubiksCube()
    : m_Mesh(nullptr), m_Shader(nullptr), m_Texture(nullptr)
{
    // Initialize graphics resources
    // Note: In a real engine, these might be passed in or managed by a Resource Manager
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

    // Loop from -1 to 1 on all axes to build the 3x3x3 grid
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            for (int z = -1; z <= 1; z++)
            {
                Cubie newCubie;
                newCubie.id = idCounter++;
                newCubie.currentGridPos = glm::ivec3(x, y, z);
                // Set initial local transformation (identity)
                newCubie.localRotation = glm::mat4(1.0f);

                // Determine colors based on grid position
                SetupStickers(newCubie, x, y, z);

                m_Cubies.push_back(newCubie);
            }
        }
    }
}

void RubiksCube::SetupStickers(Cubie& cubie, int x, int y, int z)
{
    // Reset all to None first
    for (int i = 0; i < 6; i++) 
        cubie.stickers[i] = StickerColor::None;

    // Assign colors only if the face is on the outer shell of the Rubik's cube
    
    // Right (+X)
    if (x == 1)  cubie.stickers[(int)Face::PosX] = StickerColor::Red;
    
    // Left (-X)
    if (x == -1) cubie.stickers[(int)Face::NegX] = StickerColor::Orange;
    
    // Top (+Y)
    if (y == 1)  cubie.stickers[(int)Face::PosY] = StickerColor::White;
    
    // Bottom (-Y)
    if (y == -1) cubie.stickers[(int)Face::NegY] = StickerColor::Yellow;
    
    // Front (+Z)
    if (z == 1)  cubie.stickers[(int)Face::PosZ] = StickerColor::Blue;
    
    // Back (-Z)
    if (z == -1) cubie.stickers[(int)Face::NegZ] = StickerColor::Green;
}

glm::vec3 RubiksCube::GetInitialPosition(int x, int y, int z) const
{
    return glm::vec3(x * SPACING, y * SPACING, z * SPACING);
}

void RubiksCube::Draw(const glm::mat4& viewProj, const glm::mat4& globalModel)
{
    m_Shader->Bind();
    m_Texture->Bind(0);
    m_Shader->SetUniform1i("u_Texture", 0);
    m_Mesh->Bind();

    // שינוי: במקום לולאות מקוננות שמייצרות מיקומים, עוברים על הקוביות הקיימות
    // ומשתמשים במיקום (GridPos) השמור בתוכן.
    for (const auto& cubie : m_Cubies)
    {
        // 1. שליפת המיקום הלוגי מתוך הקובייה
        int x = cubie.currentGridPos.x;
        int y = cubie.currentGridPos.y;
        int z = cubie.currentGridPos.z;

        // 2. חישוב המיקום הפיזי בעולם לפי הגריד (Spacing * Index)
        glm::vec3 currentPos = GetInitialPosition(x, y, z);

        // --- מכאן והלאה הקוד זהה למקור ---

        // כרגע אין אנימציה פרטנית, מטריצת יחידה
        glm::mat4 animRot = glm::mat4(1.0f);

        // חישוב המודל הסופי של הקובייה הבודדת
        glm::mat4 model = globalModel * cubie.BuildModel(currentPos, animRot, 1.0f);
        glm::mat4 mvp = viewProj * model;

        m_Shader->SetUniformMat4f("u_MVP", mvp);

        // ציור 6 הפאות
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
                colorVec = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f); // פלסטיק שחור
            else
                colorVec = StickerToVec4(sc);

            m_Shader->SetUniform4f("u_Color", colorVec);

            // 6 אינדקסים לכל פאה
            const void* offset = (const void*)(group * 6 * sizeof(unsigned int));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, offset);
        }
    }
}

// בתוך RubiksCube.cpp

// עדכן את הפונקציה Draw:
void RubiksCube::Draw(const glm::mat4& viewProj, const glm::mat4& globalModel, 
                      bool isAnimating, glm::vec3 animAxis, float animDeg)
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
        
        // --- השינוי מתחיל כאן ---
        
        glm::mat4 animRot = glm::mat4(1.0f);

        if (isAnimating)
        {
            // בדיקה: האם הקובייה הזו שייכת לשכבה שמסתובבת?
            // הסבר: אם הציר הוא (1,0,0) כלומר R, והקובייה ב-(1,1,0), אז 1*1 = 1 (חיובי) -> תסתובב.
            // אם הקובייה ב-(-1,1,0), אז -1*1 = -1 (שלילי) -> לא תסתובב.
            glm::vec3 fGridPos = glm::vec3(x, y, z);
            
            // משתמשים ב-dot product כדי לבדוק התאמה לציר
            if (glm::dot(fGridPos, animAxis) > 0.1f) 
            {
                animRot = glm::rotate(glm::mat4(1.0f), glm::radians(animDeg), animAxis);
            }
        }
        
        // --- השינוי מסתיים כאן ---

        glm::mat4 model = globalModel * cubie.BuildModel(currentPos, animRot, 1.0f);
        glm::mat4 mvp = viewProj * model;

        m_Shader->SetUniformMat4f("u_MVP", mvp);

        // ... המשך הקוד של ציור הפאות נשאר ללא שינוי ...
        Face groupToFace[6] = { Face::PosZ, Face::NegZ, Face::PosX, Face::NegX, Face::PosY, Face::NegY };
        for (int group = 0; group < 6; ++group) {
             Face f = groupToFace[group];
             StickerColor sc = cubie.stickers[(int)f];
             glm::vec4 colorVec = (sc == StickerColor::None) ? glm::vec4(0.05f, 0.05f, 0.05f, 1.0f) : StickerToVec4(sc);
             m_Shader->SetUniform4f("u_Color", colorVec);
             const void* offset = (const void*)(group * 6 * sizeof(unsigned int));
             glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, offset);
        }
    }
}

void RubiksCube::FinishTurn(glm::vec3 axis, float deg)
{
    // יצירת מטריצת הסיבוב שהשתמשנו בה באנימציה
    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(deg), axis);

    for (auto& cubie : m_Cubies)
    {
        // בודקים אם הקובייה הזו נמצאת בשכבה שהסתובבה
        glm::vec3 p = glm::vec3(cubie.currentGridPos);
        
        // אם הקובייה נמצאת בכיוון הציר (למשל x=1 והציר הוא 1,0,0)
        if (glm::dot(p, axis) > 0.1f) 
        {
            // 1. עדכון המיקום הלוגי (x,y,z)
            // מכפילים במטריצה ומעגלים כדי לקבל מספרים שלמים (למשל 1, 0, -1)
            glm::vec3 newPosf = glm::vec3(rot * glm::vec4(p, 1.0f));
            cubie.currentGridPos = glm::ivec3(glm::round(newPosf));

            // 2. עדכון האוריינטציה (שמירת הסיבוב)
            // אנחנו "אופים" את הסיבוב החדש לתוך הסיבוב המקומי של הקובייה
            cubie.localRotation = rot * cubie.localRotation;
        }
    }
}