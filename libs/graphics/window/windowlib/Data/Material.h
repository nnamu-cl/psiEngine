#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// Color mode selection
enum class ColorMode
{
    VertexColor,  // Use per-vertex colors from mesh
    ObjectColor   // Use single color for entire object
};

// Shading mode selection
enum class ShadingMode
{
    Lit,      // Uses lit shader with lighting calculations
    Unlit     // Uses unlit shader (flat color/emissive)
};

// Blend mode for rendering
enum class BlendMode
{
    Opaque,       // No blending, fully opaque
    Transparent,  // Alpha blending
    Additive,     // Additive blending (for glowing effects)
    Multiply      // Multiplicative blending (for shadows/decals)
};

// Material properties for rendering
struct MaterialProperties
{
    // Tier 1: Core functionality
    ColorMode colorMode = ColorMode::VertexColor;  // How to source color data
    glm::vec4 objectColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // Solid color for ObjectColor mode
    ShadingMode shadingMode = ShadingMode::Lit;
    float emissionIntensity = 1.0f;  // Brightness multiplier (1.0 = normal, >1.0 = brighter)

    // Tier 2: Highly recommended
    glm::vec3 tintColor = glm::vec3(1.0f);  // Color multiplier for vertex colors
    BlendMode blendMode = BlendMode::Opaque;

    // Tier 3: Nice-to-have
    float alphaCutoff = 0.5f;  // Alpha threshold for alpha testing (0.0-1.0)
    bool doubleSided = false;  // Render both front and back faces
};
