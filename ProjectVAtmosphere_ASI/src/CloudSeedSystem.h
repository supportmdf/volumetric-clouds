#pragma once

#include "CloudTypes.h"

namespace PVA
{
    class CloudSeedSystem
    {
    public:
        void Initialize(unsigned int worldSeed);

        unsigned int MakeRegionSeed(int gridX, int gridY) const;
        float Value01(unsigned int seed) const;
        float Noise2D(float x, float y, unsigned int seed) const;
        float FractalNoise2D(float x, float y, unsigned int seed, int octaves) const;
        float Worley2D(float x, float y, unsigned int seed) const;

    private:
        unsigned int Hash(unsigned int value) const;
        unsigned int HashCoords(int x, int y, unsigned int seed) const;
        float Smooth(float value) const;

        unsigned int m_worldSeed = 0U;
    };
}
