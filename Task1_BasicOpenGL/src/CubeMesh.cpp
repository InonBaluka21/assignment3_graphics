#include "CubeMesh.h"
#include "Debugger.h"

#include <glad/glad.h>

// 24 vertices (4 per face), each vertex: pos(3), color(3), uv(2) = 8 floats
static float cubeVertices[] = {
    // +Z (front)
    -1.f,-1.f, 1.f,   1,1,1,   0,0,
     1.f,-1.f, 1.f,   1,1,1,   1,0,
     1.f, 1.f, 1.f,   1,1,1,   1,1,
    -1.f, 1.f, 1.f,   1,1,1,   0,1,

    // -Z (back)
     1.f,-1.f,-1.f,   1,1,1,   0,0,
    -1.f,-1.f,-1.f,   1,1,1,   1,0,
    -1.f, 1.f,-1.f,   1,1,1,   1,1,
     1.f, 1.f,-1.f,   1,1,1,   0,1,

    // +X (right)
     1.f,-1.f, 1.f,   1,1,1,   0,0,
     1.f,-1.f,-1.f,   1,1,1,   1,0,
     1.f, 1.f,-1.f,   1,1,1,   1,1,
     1.f, 1.f, 1.f,   1,1,1,   0,1,

    // -X (left)
    -1.f,-1.f,-1.f,   1,1,1,   0,0,
    -1.f,-1.f, 1.f,   1,1,1,   1,0,
    -1.f, 1.f, 1.f,   1,1,1,   1,1,
    -1.f, 1.f,-1.f,   1,1,1,   0,1,

    // +Y (top)
    -1.f, 1.f, 1.f,   1,1,1,   0,0,
     1.f, 1.f, 1.f,   1,1,1,   1,0,
     1.f, 1.f,-1.f,   1,1,1,   1,1,
    -1.f, 1.f,-1.f,   1,1,1,   0,1,

    // -Y (bottom)
    -1.f,-1.f,-1.f,   1,1,1,   0,0,
     1.f,-1.f,-1.f,   1,1,1,   1,0,
     1.f,-1.f, 1.f,   1,1,1,   1,1,
    -1.f,-1.f, 1.f,   1,1,1,   0,1,
};

static unsigned int cubeIndices[] = {
    0,1,2,  2,3,0,        // front
    4,5,6,  6,7,4,        // back
    8,9,10, 10,11,8,      // right
    12,13,14, 14,15,12,   // left
    16,17,18, 18,19,16,   // top
    20,21,22, 22,23,20    // bottom
};

CubeMesh::CubeMesh()
    : m_VAO()
    , m_VBO(cubeVertices, sizeof(cubeVertices))
    , m_EBO(cubeIndices, sizeof(cubeIndices))
{
    VertexBufferLayout layout;
    layout.Push<float>(3); // positions
    layout.Push<float>(3); // colors
    layout.Push<float>(2); // texCoords
    m_VAO.AddBuffer(m_VBO, layout);
}

void CubeMesh::Bind() const
{
    m_VAO.Bind();
    m_EBO.Bind();
}

void CubeMesh::Unbind() const
{
    m_EBO.Unbind();
    m_VAO.Unbind();
}

unsigned int CubeMesh::GetIndexCount() const
{
    return m_EBO.GetCount();
}

void CubeMesh::Draw() const
{
    Bind();
    GLCall(glDrawElements(GL_TRIANGLES, m_EBO.GetCount(), GL_UNSIGNED_INT, nullptr));
}
