#include "Mesh.h"
#include <generator/BoxMesh.hpp>
#include <generator/TriangleMesh.hpp>

namespace {
    // Helper function to convert generator mesh to our Mesh format
    template<typename GeneratorMesh>
    Mesh convertFromGenerator(GeneratorMesh&& genMesh, const glm::vec4& color = glm::vec4{1.0f})
    {
        Mesh mesh;

        // Extract vertices
        auto vertexGen = genMesh.vertices();
        while (!vertexGen.done()) {
            auto v = vertexGen.generate();
            mesh.vertices.push_back({
                glm::vec3(static_cast<float>(v.position[0]),
                         static_cast<float>(v.position[1]),
                         static_cast<float>(v.position[2])),
                glm::vec3(static_cast<float>(v.normal[0]),
                         static_cast<float>(v.normal[1]),
                         static_cast<float>(v.normal[2])),
                glm::vec2(static_cast<float>(v.texCoord[0]),
                         static_cast<float>(v.texCoord[1])),
                color
            });
            vertexGen.next();
        }

        // Extract indices
        auto triangleGen = genMesh.triangles();
        while (!triangleGen.done()) {
            auto tri = triangleGen.generate();
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[0]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[1]));
            mesh.indices.push_back(static_cast<uint32_t>(tri.vertices[2]));
            triangleGen.next();
        }

        return mesh;
    }
}

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
    // Create a triangle with radius 1.0, 1 segment (simple triangle)
    return convertFromGenerator(generator::TriangleMesh(1.0, 1));
}

// ---------------------------------------------------------------------------
// Unit cube: axis-aligned, centred at origin, extents [-1,1] on each axis.
// Each face has its own quad (4 vertices) so normals are hard-edged.
// Winding is counter-clockwise when viewed from outside (standard front-face).
// ---------------------------------------------------------------------------
Mesh MeshTable::unitCube()
{
    // BoxMesh takes half-extents, so {1, 1, 1} gives a box from [-1,-1,-1] to [1,1,1]
    // Segments {1, 1, 1} gives us the simplest box with hard edges
    return convertFromGenerator(generator::BoxMesh({1.0, 1.0, 1.0}, {1, 1, 1}));
}
