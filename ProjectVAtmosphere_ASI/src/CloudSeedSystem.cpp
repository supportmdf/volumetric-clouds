#include "CloudSeedSystem.h"

#include <cmath>

namespace PVA
{
    void CloudSeedSystem::Initialize(const unsigned int worldSeed)
    {
        m_worldSeed = worldSeed;
    }

    unsigned int CloudSeedSystem::MakeRegionSeed(const int gridX, const int gridY) const
    {
        return HashCoords(gridX, gridY, m_worldSeed);
    }

    float CloudSeedSystem::Value01(const unsigned int seed) const
    {
        return static_cast<float>(Hash(seed) & 0x00FFFFFFU) * (1.0f / 16777215.0f);
    }

    float CloudSeedSystem::Noise2D(const float x, const float y, const unsigned int seed) const
    {
        const int ix = static_cast<int>(std::floor(x));
        const int iy = static_cast<int>(std::floor(y));
        const float fx = x - static_cast<float>(ix);
        const float fy = y - static_cast<float>(iy);
        const float sx = Smooth(fx);
        const float sy = Smooth(fy);

        const float n00 = Value01(HashCoords(ix, iy, seed));
        const float n10 = Value01(HashCoords(ix + 1, iy, seed));
        const float n01 = Value01(HashCoords(ix, iy + 1, seed));
        const float n11 = Value01(HashCoords(ix + 1, iy + 1, seed));

        const float nx0 = Lerp(n00, n10, sx);
        const float nx1 = Lerp(n01, n11, sx);
        return Lerp(nx0, nx1, sy);
    }

    float CloudSeedSystem::FractalNoise2D(const float x, const float y, const unsigned int seed, const int octaves) const
    {
        float amplitude = 0.5f;
        float frequency = 1.0f;
        float value = 0.0f;
        float normalization = 0.0f;

        for (int octave = 0; octave < octaves; ++octave)
        {
            value += Noise2D(x * frequency, y * frequency, seed + static_cast<unsigned int>(octave * 131U)) * amplitude;
            normalization += amplitude;
            frequency *= 2.03f;
            amplitude *= 0.52f;
        }

        return normalization > 0.0f ? value / normalization : 0.0f;
    }

    float CloudSeedSystem::Worley2D(const float x, const float y, const unsigned int seed) const
    {
        const int cellX = static_cast<int>(std::floor(x));
        const int cellY = static_cast<int>(std::floor(y));
        float closestSq = 1000000.0f;

        for (int yy = -1; yy <= 1; ++yy)
        {
            for (int xx = -1; xx <= 1; ++xx)
            {
                const int px = cellX + xx;
                const int py = cellY + yy;
                const unsigned int h = HashCoords(px, py, seed);
                const float jitterX = Value01(h);
                const float jitterY = Value01(Hash(h + 17U));
                const float dx = (static_cast<float>(px) + jitterX) - x;
                const float dy = (static_cast<float>(py) + jitterY) - y;
                const float distSq = (dx * dx) + (dy * dy);
                closestSq = Min(closestSq, distSq);
            }
        }

        return Saturate(std::sqrt(closestSq));
    }

    unsigned int CloudSeedSystem::Hash(unsigned int value) const
    {
        value ^= value >> 16U;
        value *= 0x7FEB352DU;
        value ^= value >> 15U;
        value *= 0x846CA68BU;
        value ^= value >> 16U;
        return value;
    }

    unsigned int CloudSeedSystem::HashCoords(const int x, const int y, const unsigned int seed) const
    {
        const unsigned int ux = static_cast<unsigned int>(x) * 0x8DA6B343U;
        const unsigned int uy = static_cast<unsigned int>(y) * 0xD8163841U;
        return Hash(ux ^ uy ^ seed ^ m_worldSeed);
    }

    float CloudSeedSystem::Smooth(const float value) const
    {
        return value * value * value * (value * ((value * 6.0f) - 15.0f) + 10.0f);
    }
}
