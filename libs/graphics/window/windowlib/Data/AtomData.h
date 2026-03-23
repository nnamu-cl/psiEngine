#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <limits>
#include <algorithm>

// Raw atomic state — independent of any visualizer.
struct AtomData
{
    int n = 1;   // principal quantum number (>= 1)
    int l = 0;   // angular momentum (0 <= l < n)
    int m = 0;   // magnetic quantum number (-l <= m <= l)

    // Visual scaling factor: multiplies the Bohr radius (a0 = 1 in atomic units)
    // so the orbital fits nicely in the scene.
    float bohrScale = 2.0f;

    mutable bool needsGPUUpdate = false;
    mutable glm::vec3 lastUploadedPos = glm::vec3(std::numeric_limits<float>::max());

    // Enforce quantum number validity rules
    void validate()
    {
        n = std::max(n, 1);
        l = std::clamp(l, 0, n - 1);
        m = std::clamp(m, -l, l);
    }

    // Bounding radius in scaled atomic units — orbital extent ~ 2 * n^2 * bohrScale
    float boundingRadius() const
    {
        return 2.0f * static_cast<float>(n * n) * bohrScale;
    }
};
