// Orbital operations: geometry expansion and fragment particle-grid rendering
// Renders hydrogen atom orbital probability densities |ψ(n,l,m)|² as dot clouds
import common;

static const float PI = 3.14159265358979323846;

// ============================================================
// Hash utilities for deterministic pseudo-random particle placement
// ============================================================
uint hashUint(uint x)
{
    x ^= x >> 16u;
    x *= 0x45d9f3bu;
    x ^= x >> 16u;
    x *= 0x45d9f3bu;
    x ^= x >> 16u;
    return x;
}

uint hash3D(int3 p)
{
    uint h = uint(p.x + 10000) * 73856093u
           ^ uint(p.y + 10000) * 19349663u
           ^ uint(p.z + 10000) * 83492791u;
    return hashUint(h);
}

float hashFloat(uint h, uint salt)
{
    return float(hashUint(h ^ (salt * 2654435761u)) & 0x00FFFFFFu) / float(0x00FFFFFFu);
}

// ============================================================
// Fire heatmap: Black → Purple → Red → Orange → Yellow → White
// ============================================================
float3 heatmapFire(float t)
{
    t = saturate(t);
    // 6 color stops at t = 0, 0.2, 0.4, 0.6, 0.8, 1.0
    float s = t * 5.0;
    int i = int(s);
    float f = s - float(i);

    if (i >= 5) return float3(1.0, 1.0, 1.0);

    float3 c0, c1;
    if      (i == 0) { c0 = float3(0.0, 0.0, 0.0);  c1 = float3(0.5, 0.0, 0.99); }
    else if (i == 1) { c0 = float3(0.5, 0.0, 0.99); c1 = float3(0.8, 0.0, 0.0);  }
    else if (i == 2) { c0 = float3(0.8, 0.0, 0.0);  c1 = float3(1.0, 0.5, 0.0);  }
    else if (i == 3) { c0 = float3(1.0, 0.5, 0.0);  c1 = float3(1.0, 1.0, 0.0);  }
    else             { c0 = float3(1.0, 1.0, 0.0);  c1 = float3(1.0, 1.0, 1.0);  }

    return lerp(c0, c1, f);
}

// ============================================================
// Associated Laguerre polynomial L_p^k(x) via 3-term recurrence
// ============================================================
float laguerre(int p, int k, float x)
{
    if (p == 0) return 1.0;
    if (p == 1) return 1.0 + float(k) - x;

    float Lm1 = 1.0 + float(k) - x;  // L_1^k
    float Lm2 = 1.0;                   // L_0^k

    for (int j = 1; j < p; j++)
    {
        float Lnew = ((2.0 * float(j) + 1.0 + float(k) - x) * Lm1
                     - (float(j) + float(k)) * Lm2) / float(j + 1);
        Lm2 = Lm1;
        Lm1 = Lnew;
    }
    return Lm1;
}

// ============================================================
// Associated Legendre polynomial P_l^|m|(x) via recurrence
// ============================================================
float legendreP(int l, int absM, float x)
{
    // P_m^m(x) = (-1)^m (2m-1)!! (1-x^2)^(m/2)
    float pmm = 1.0;
    if (absM > 0)
    {
        float somx2 = sqrt(max(1.0 - x * x, 0.0));
        float fact = 1.0;
        for (int i = 1; i <= absM; i++)
        {
            pmm *= -fact * somx2;
            fact += 2.0;
        }
    }

    if (l == absM)
        return pmm;

    // P_{m+1}^m(x) = x(2m+1) P_m^m
    float pmm1 = x * (2.0 * float(absM) + 1.0) * pmm;
    if (l == absM + 1)
        return pmm1;

    // Recurrence: (l-m)P_l^m = x(2l-1)P_{l-1}^m - (l+m-1)P_{l-2}^m
    float pll = 0.0;
    for (int ll = absM + 2; ll <= l; ll++)
    {
        pll = (x * (2.0 * float(ll) - 1.0) * pmm1 - (float(ll) + float(absM) - 1.0) * pmm)
              / (float(ll) - float(absM));
        pmm = pmm1;
        pmm1 = pll;
    }
    return pll;
}

// ============================================================
// Factorial (small values, n <= 20)
// ============================================================
float factorial(int n)
{
    float result = 1.0;
    for (int i = 2; i <= n; i++)
        result *= float(i);
    return result;
}

// ============================================================
// Real spherical harmonic Y_l^m(theta, phi) — signed value
// ============================================================
float realSphericalHarmonic(int l, int m, float cosTheta, float phi)
{
    int absM = abs(m);
    float plm = legendreP(l, absM, cosTheta);

    float norm = sqrt((2.0 * float(l) + 1.0) / (4.0 * PI)
                      * factorial(l - absM) / factorial(l + absM));

    if (m > 0)
        return sqrt(2.0) * norm * plm * cos(float(m) * phi);
    else if (m < 0)
        return sqrt(2.0) * norm * plm * sin(float(absM) * phi);
    else
        return norm * plm;
}

// ============================================================
// Radial wavefunction R_{n,l}(r) with a0 = bohrScale
// ============================================================
float radialWavefunction(int n, int l, float r, float bohrScale)
{
    float a0 = bohrScale;
    float rho = 2.0 * r / (float(n) * a0);

    // Normalization: N = (2/(n*a0))^(3/2) * sqrt((n-l-1)! / (2n*(n+l)!))
    float twoOverNa = 2.0 / (float(n) * a0);
    float norm = pow(twoOverNa, 1.5)
               * sqrt(factorial(n - l - 1) / (2.0 * float(n) * factorial(n + l)));

    float L = laguerre(n - l - 1, 2 * l + 1, rho);

    return norm * pow(rho, float(l)) * L * exp(-rho * 0.5);
}

// ============================================================
// Evaluate signed wavefunction ψ at a point in local space
// ============================================================
float evaluatePsi(float3 pos, int n, int l, int m, float bohrScale)
{
    float r = length(pos);
    if (r < 1e-8) r = 1e-8;

    // Y is the polar axis (up), matching standard 3D engine convention
    float cosTheta = pos.y / r;
    float phi = atan2(pos.z, pos.x);

    float R = radialWavefunction(n, l, r, bohrScale);
    float Y = realSphericalHarmonic(l, m, cosTheta, phi);

    return R * Y;
}

// ============================================================
// Ray-sphere intersection: returns (tNear, tFar), or (-1,-1) if miss
// ============================================================
float2 raySphereIntersect(float3 rayOrigin, float3 rayDir, float3 center, float radius)
{
    float3 oc = rayOrigin - center;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - c;

    if (disc < 0.0)
        return float2(-1.0, -1.0);

    float sqrtD = sqrt(disc);
    return float2(-b - sqrtD, -b + sqrtD);
}

// ============================================================
// Geometry shader: emit billboard quad for orbital volume
// Uses world-space position from vertex shader for reliable data passing
// ============================================================
void emitOrbitalQuad(
    VertexToGeom centerIn,
    LineObjectData objData,
    LinePushConstants pushConstants,
    inout TriangleStream<FragmentInput> outputStream)
{
    float4 clipCenter = centerIn.position;
    float2 ndcCenter = clipCenter.xy / clipCenter.w;

    float3 worldCenter = centerIn.worldPos;

    float boundingRadius = objData.radius;

    // Perspective-correct radius in NDC
    float radiusNDC = boundingRadius * pushConstants.projScale / clipCenter.w;
    float radiusX = radiusNDC / pushConstants.aspectRatio;
    float radiusY = radiusNDC;

    // Camera axes from the viewProj matrix (rows of the view matrix portion)
    float4x4 M = pushConstants.viewProj;
    float3 right   = normalize(float3(M[0][0], M[0][1], M[0][2]));
    float3 up      = normalize(float3(M[1][0], M[1][1], M[1][2]));

    // World-space billboard corners
    float3 cornerBL = worldCenter - right * boundingRadius - up * boundingRadius;
    float3 cornerTL = worldCenter - right * boundingRadius + up * boundingRadius;
    float3 cornerBR = worldCenter + right * boundingRadius - up * boundingRadius;
    float3 cornerTR = worldCenter + right * boundingRadius + up * boundingRadius;

    FragmentInput v;
    v.objectType    = 2;
    v.color         = objData.positiveColor;
    v.lineStyle     = 0;
    v.dashLength    = 0;
    v.gapLength     = 0;
    v.antiAlias     = 0;
    v.smoothness    = 0;
    v.lit           = 0;
    v.distance      = 0;
    v.worldCenter   = worldCenter;
    v.orbN          = objData.orbN;
    v.orbL          = objData.orbL;
    v.orbM          = objData.orbM;
    v.bohrScale     = objData.bohrScale;
    v.densityScale  = objData.densityScale;
    v.orbStepCount  = objData.stepCount;
    v.positiveColor = objData.positiveColor;
    v.negativeColor = objData.negativeColor;
    v.animationSpeed = objData.animationSpeed;

    // Triangle 1: BL, TL, BR
    v.position = float4((ndcCenter + float2(-radiusX, -radiusY)) * clipCenter.w, clipCenter.z, clipCenter.w);
    v.uv = float2(-1.0, -1.0);
    v.worldPos = cornerBL;
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(-radiusX, radiusY)) * clipCenter.w, clipCenter.z, clipCenter.w);
    v.uv = float2(-1.0, 1.0);
    v.worldPos = cornerTL;
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(radiusX, -radiusY)) * clipCenter.w, clipCenter.z, clipCenter.w);
    v.uv = float2(1.0, -1.0);
    v.worldPos = cornerBR;
    outputStream.Append(v);

    outputStream.RestartStrip();

    // Triangle 2: TL, TR, BR
    v.position = float4((ndcCenter + float2(-radiusX, radiusY)) * clipCenter.w, clipCenter.z, clipCenter.w);
    v.uv = float2(-1.0, 1.0);
    v.worldPos = cornerTL;
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(radiusX, radiusY)) * clipCenter.w, clipCenter.z, clipCenter.w);
    v.uv = float2(1.0, 1.0);
    v.worldPos = cornerTR;
    outputStream.Append(v);

    v.position = float4((ndcCenter + float2(radiusX, -radiusY)) * clipCenter.w, clipCenter.z, clipCenter.w);
    v.uv = float2(1.0, -1.0);
    v.worldPos = cornerBR;
    outputStream.Append(v);

    outputStream.RestartStrip();
}

// ============================================================
// Fragment shader: ray march through a pseudo-random particle grid
// Each grid cell may contain a dot; acceptance is proportional to |ψ|²
// Produces a particle-cloud look with a single draw call per atom
// ============================================================
float4 shadeOrbital(FragmentInput input, LinePushConstants pushConstants)
{
    float3 camPos = pushConstants.cameraPos;
    float3 fragWorldPos = input.worldPos;
    float3 center = input.worldCenter;

    float3 rayDir = normalize(fragWorldPos - camPos);
    float3 rayOrigin = camPos;

    float bRadius = input.bohrScale * 2.0 * float(input.orbN * input.orbN);

    float2 tHit = raySphereIntersect(rayOrigin, rayDir, center, bRadius);

    if (tHit.x < 0.0 && tHit.y < 0.0)
        discard;

    float tNear = max(tHit.x, 0.001);
    float tFar  = tHit.y;

    if (tNear >= tFar)
        discard;

    int n = int(input.orbN);
    int l = int(input.orbL);
    int m = input.orbM;
    float bScale = input.bohrScale;
    float dScale = input.densityScale;
    float animSpeed = input.animationSpeed;
    float time = pushConstants.time;

    // Probability current rotation angle: phi_dot = m * hbar / (m_e * r * sinTheta)
    // Simplified to a uniform angular offset for visual effect
    float rotAngle = float(m) * animSpeed * time * 0.5;

    // Grid cell size — controls dot density
    // More cells = more candidate particles = denser cloud
    int steps = int(input.orbStepCount);
    float cellSize = (bRadius * 2.0) / float(steps);
    float dotRadius = cellSize * 0.45;

    // Step through ray at cell-size intervals so we visit each cell once
    float stepSize = cellSize;
    int numSteps = int(ceil((tFar - tNear) / stepSize));
    numSteps = min(numSteps, 512);

    // Peak density for normalization so acceptance is in [0,1]
    // For s-orbitals evaluate near origin; for l>0 at characteristic radius
    float rPeak = (l == 0) ? 0.05 * bScale : float(l) * float(n) * bScale * 0.5;
    float psiPeak = evaluatePsi(float3(0.0, rPeak, 0.0), n, l, m, bScale);
    float peakDensity = psiPeak * psiPeak;
    if (peakDensity < 1e-20) peakDensity = 1e-20;

    float3 accumColor = float3(0.0);
    float accumAlpha = 0.0;

    int3 lastCell = int3(-999999);

    for (int i = 0; i < numSteps; i++)
    {
        if (accumAlpha > 0.97)
            break;

        float t = tNear + (float(i) + 0.5) * stepSize;
        float3 localPos = (rayOrigin + rayDir * t) - center;

        // Which grid cell are we in?
        int3 cell = int3(floor(localPos / cellSize));

        // Skip if same cell as last step (avoid double-counting)
        if (cell.x == lastCell.x && cell.y == lastCell.y && cell.z == lastCell.z)
            continue;
        lastCell = cell;

        // Hash this cell to get a deterministic random particle position
        uint h = hash3D(cell);

        // Random offset within cell → particle position
        float3 cellCenter = (float3(cell) + 0.5) * cellSize;
        float3 particleLocal = cellCenter + float3(
            hashFloat(h, 0u) - 0.5,
            hashFloat(h, 1u) - 0.5,
            hashFloat(h, 2u) - 0.5
        ) * cellSize * 0.75;

        // Skip if outside bounding sphere
        if (length(particleLocal) > bRadius)
            continue;

        // Rotate particle around Y-axis (polar axis) for probability current animation
        // This makes m != 0 orbitals visibly orbit
        float cosR = cos(rotAngle);
        float sinR = sin(rotAngle);
        float3 rotatedLocal = float3(
            particleLocal.x * cosR - particleLocal.z * sinR,
            particleLocal.y,
            particleLocal.x * sinR + particleLocal.z * cosR
        );

        // Evaluate |ψ|² at the rotated particle position
        float psi = evaluatePsi(rotatedLocal, n, l, m, bScale);
        float density = psi * psi;
        float normDensity = density / peakDensity;

        // Rejection sampling: accept particle if density beats random threshold
        float roll = hashFloat(h, 3u);
        if (normDensity * dScale < roll)
            continue;

        // Closest approach of ray to particle center (use rotated position)
        float3 particleWorld = rotatedLocal + center;
        float3 op = particleWorld - rayOrigin;
        float tClosest = dot(op, rayDir);
        float3 closest = rayOrigin + rayDir * tClosest;
        float dist = length(particleWorld - closest);

        // Only render if ray passes within dot radius
        if (dist > dotRadius)
            continue;

        // Smooth circle falloff
        float r2 = dist / dotRadius;
        float circle = 1.0 - r2 * r2;
        circle = circle * circle; // sharper edge

        // Fire heatmap color based on normalized probability density
        // normDensity is already in [0,1] — no need for pow(5,n) compensation
        float intensity = saturate(normDensity * 1.5);
        float3 dotColor = heatmapFire(intensity);

        // Low per-dot opacity so the ray penetrates deep into the cloud,
        // revealing the internal lobe structure rather than just the outer shell
        float alpha = circle * 0.15;

        // Front-to-back compositing
        float w = (1.0 - accumAlpha) * alpha;
        accumColor += w * dotColor;
        accumAlpha += w;
    }

    if (accumAlpha < 0.003)
        discard;

    return float4(accumColor, accumAlpha);
}
