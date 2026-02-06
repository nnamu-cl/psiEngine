#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <string>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Vertex layout that will be mirrored in the vertex shader.
// Offsets are tightly packed: pos(12) + normal(12) + texCoord(8) = 32 bytes.
// ---------------------------------------------------------------------------
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

// ---------------------------------------------------------------------------
// A single mesh: its vertex and index data lives on the CPU here.
// Later stages will upload these into GPU buffers.
// ---------------------------------------------------------------------------
struct Mesh
{
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
};

// ---------------------------------------------------------------------------
// Registry of named meshes.  Meshes are stored by value so the table owns
// the data.  Indices returned by add() / get() are stable as long as no
// mesh is removed (which we don't support yet).
// ---------------------------------------------------------------------------
class MeshTable
{
public:
    // Adds a mesh and returns its index.  If a mesh with the same name already
    // exists the new data replaces it and the same index is returned.
    uint32_t add(const std::string& name, Mesh mesh);

    // Returns a pointer to the mesh, or nullptr if not found.
    const Mesh* get(uint32_t index) const;
    const Mesh* get(const std::string& name) const;

    // Number of meshes currently in the table.
    uint32_t count() const { return static_cast<uint32_t>(m_meshes.size()); }

    // ---------------------------------------------------------------------------
    // Built-in primitive generators.  Each returns a fully populated Mesh
    // (positions, normals, texCoords, indices) ready to be added to a table.
    // ---------------------------------------------------------------------------
    static Mesh unitCube();
    static Mesh unitTriangle();

private:
    std::vector<Mesh>                    m_meshes;
    std::unordered_map<std::string, uint32_t> m_nameToIndex;
};
