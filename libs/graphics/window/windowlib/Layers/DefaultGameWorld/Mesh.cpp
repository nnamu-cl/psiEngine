#include "Mesh.h"

uint32_t MeshTable::add(const std::string& name, Mesh mesh)
{
    auto it = m_nameToIndex.find(name);
    if (it != m_nameToIndex.end())
    {
        m_meshes[it->second] = std::move(mesh);
        return it->second;
    }

    uint32_t index = static_cast<uint32_t>(m_meshes.size());
    m_meshes.push_back(std::move(mesh));
    m_nameToIndex[name] = index;
    return index;
}

const Mesh* MeshTable::get(uint32_t index) const
{
    if (index >= m_meshes.size()) return nullptr;
    return &m_meshes[index];
}

const Mesh* MeshTable::get(const std::string& name) const
{
    auto it = m_nameToIndex.find(name);
    if (it == m_nameToIndex.end()) return nullptr;
    return &m_meshes[it->second];
}

// ---------------------------------------------------------------------------
// Unit triangle: a single triangle in the XY plane, facing +Z.
// Useful as the simplest possible draw call to verify the pipeline end-to-end.
// ---------------------------------------------------------------------------
Mesh MeshTable::unitTriangle()
{
    Mesh m;
    m.vertices = {
        { glm::vec3{ 0.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.5f, 0.0f } },
        { glm::vec3{-1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 0.0f, 1.0f } },
        { glm::vec3{ 1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f }, glm::vec2{ 1.0f, 1.0f } },
    };
    m.indices = { 0, 1, 2 };
    return m;
}

// ---------------------------------------------------------------------------
// Unit cube: axis-aligned, centred at origin, extents [-1,1] on each axis.
// Each face has its own quad (4 vertices) so normals are hard-edged.
// Winding is counter-clockwise when viewed from outside (standard front-face).
// ---------------------------------------------------------------------------
Mesh MeshTable::unitCube()
{
    // Each face: 4 vertices, 2 triangles (indices 0,1,2 and 0,2,3).
    // Layout per block: positions, shared normal, UV corners.
    struct FaceData {
        glm::vec3 verts[4];
        glm::vec3 normal;
    };

    constexpr FaceData faces[6] = {
        // +Z (front)
        { { { -1,-1, 1 }, {  1,-1, 1 }, {  1, 1, 1 }, { -1, 1, 1 } }, { 0, 0, 1 } },
        // -Z (back)
        { { {  1,-1,-1 }, { -1,-1,-1 }, { -1, 1,-1 }, {  1, 1,-1 } }, { 0, 0,-1 } },
        // +Y (top)
        { { { -1, 1, 1 }, {  1, 1, 1 }, {  1, 1,-1 }, { -1, 1,-1 } }, { 0, 1, 0 } },
        // -Y (bottom)
        { { { -1,-1,-1 }, {  1,-1,-1 }, {  1,-1, 1 }, { -1,-1, 1 } }, { 0,-1, 0 } },
        // +X (right)
        { { {  1,-1, 1 }, {  1,-1,-1 }, {  1, 1,-1 }, {  1, 1, 1 } }, { 1, 0, 0 } },
        // -X (left)
        { { { -1,-1,-1 }, { -1,-1, 1 }, { -1, 1, 1 }, { -1, 1,-1 } }, {-1, 0, 0 } },
    };

    // UV layout: each face maps its quad to the full [0,1]x[0,1] tile.
    constexpr glm::vec2 uvs[4] = { { 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 } };

    Mesh m;
    m.vertices.reserve(24);
    m.indices.reserve(36);

    for (int f = 0; f < 6; ++f)
    {
        uint32_t base = static_cast<uint32_t>(m.vertices.size());
        for (int v = 0; v < 4; ++v)
        {
            m.vertices.push_back({ faces[f].verts[v], faces[f].normal, uvs[v] });
        }
        // Two triangles per face, CCW from outside.
        m.indices.insert(m.indices.end(), { base, base+1, base+2, base, base+2, base+3 });
    }

    return m;
}
