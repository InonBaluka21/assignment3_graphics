#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"

class CubeMesh
{
private:
    VertexArray  m_VAO;
    VertexBuffer m_VBO;
    IndexBuffer  m_EBO;

public:
    CubeMesh();
    ~CubeMesh() = default;

    // Prevent accidental copying (would double-delete GL resources)
    CubeMesh(const CubeMesh&) = delete;
    CubeMesh& operator=(const CubeMesh&) = delete;

    void Bind() const;
    void Unbind() const;

    void Draw() const;
    unsigned int GetIndexCount() const;
};
