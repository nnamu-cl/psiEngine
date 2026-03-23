#pragma once
#include <glm/vec4.hpp>

// Visualization settings for an atom's orbital probability density.
// Uses a particle-grid renderer: pseudo-random dots placed via rejection
// sampling against |ψ|², colored with a fire heatmap.
struct AtomVisualizerData
{
    // Phase-coded colors: positive / negative lobes of the wavefunction
    glm::vec4 positiveColor = glm::vec4(0.2f, 0.5f, 1.0f, 0.8f);  // blue
    glm::vec4 negativeColor = glm::vec4(1.0f, 0.3f, 0.2f, 0.8f);  // red

    // Density-to-opacity multiplier — controls how many dots pass rejection sampling
    // Higher = denser particle cloud, lower = sparser
    float densityScale = 1.0f;

    // Grid resolution along the bounding diameter — controls dot count and size
    // Higher = more but smaller dots (better quality, more GPU work)
    int stepCount = 200;

    // Probability current animation speed — particles orbit the z-axis
    // for m != 0 orbitals.  0 = frozen, 1 = physical speed.
    float animationSpeed = 1.0f;

    mutable bool needsGPUUpdate = false;
};
