#include "CloudVolumeGrid.h"

#include <cmath>

namespace PVA
{
    void CloudVolumeGrid::Initialize(const CloudRuntimeConfig& config, CloudSeedSystem* seedSystem, CloudLODSystem* lodSystem)
    {
        m_config = config;
        m_seedSystem = seedSystem;
        m_lodSystem = lodSystem;
        m_activeChunkCount = 0;
        m_refreshCursor = 0;

        for (int index = 0; index < kMaxCloudChunks; ++index)
        {
            m_chunks[index] = CloudChunk{};
        }
    }

    void CloudVolumeGrid::Update(const CloudCameraState& camera, const CloudMaterialState& material, const float deltaSeconds)
    {
        if (!camera.valid || m_seedSystem == nullptr)
        {
            return;
        }

        const int centerX = static_cast<int>(std::floor(camera.position.x / m_config.chunkSize));
        const int centerY = static_cast<int>(std::floor(camera.position.y / m_config.chunkSize));
        m_activeChunkCount = 0;

        for (int index = 0; index < kMaxCloudChunks; ++index)
        {
            m_chunks[index].active = false;
        }

        for (int y = -kCloudGridRadius; y <= kCloudGridRadius; ++y)
        {
            for (int x = -kCloudGridRadius; x <= kCloudGridRadius; ++x)
            {
                CloudChunk& chunk = GetOrCreateChunk(centerX + x, centerY + y, material);
                chunk.active = true;
                chunk.morphPhase += deltaSeconds * (0.015f + (material.turbulence * 0.055f));
                ++m_activeChunkCount;
            }
        }

        // Refresh one chunk seed blend per frame to avoid a large CPU spike when the player crosses region boundaries.
        m_refreshCursor = (m_refreshCursor + 1) % kMaxCloudChunks;
        if (m_chunks[m_refreshCursor].active)
        {
            const float n = m_seedSystem->FractalNoise2D(
                static_cast<float>(m_chunks[m_refreshCursor].gridX) * 0.19f,
                static_cast<float>(m_chunks[m_refreshCursor].gridY) * 0.19f,
                m_chunks[m_refreshCursor].seed,
                4);

            m_chunks[m_refreshCursor].coverage = Saturate((material.coverage * 0.78f) + (n * 0.34f) - 0.08f);
            m_chunks[m_refreshCursor].density = Saturate((material.density * 0.84f) + (n * 0.24f));
        }
    }

    void CloudVolumeGrid::BuildRenderInstances(const CloudCameraState& camera, const CloudMaterialState& material, CloudInstanceCPU* outInstances, const int maxInstances, int& outCount) const
    {
        outCount = 0;

        if (!camera.valid || outInstances == nullptr || maxInstances <= 0 || m_seedSystem == nullptr || m_lodSystem == nullptr)
        {
            return;
        }

        const float verticalSpan = Max(250.0f, material.cloudTopHeight - material.cloudBaseHeight);
        const float windOffset = material.timeSeconds * material.windSpeed * 2.65f;

        for (int chunkIndex = 0; chunkIndex < kMaxCloudChunks && outCount < maxInstances; ++chunkIndex)
        {
            const CloudChunk& chunk = m_chunks[chunkIndex];
            if (!chunk.active)
            {
                continue;
            }

            const float3 toChunk = Subtract(chunk.origin, camera.position);
            const float distance = Length(toChunk);
            const float facing = Dot(Normalize(toChunk), camera.forward);

            if (facing < -0.12f)
            {
                continue;
            }

            const CloudLODResult lod = m_lodSystem->Evaluate(distance, chunk.coverage, chunk.density);
            if (!lod.visible)
            {
                continue;
            }

            for (int local = 0; local < lod.instanceCount && outCount < maxInstances; ++local)
            {
                const unsigned int seed = chunk.seed + static_cast<unsigned int>(local * 977U);
                const float rx = (m_seedSystem->Value01(seed) - 0.5f) * m_config.chunkSize;
                const float ry = (m_seedSystem->Value01(seed + 19U) - 0.5f) * m_config.chunkSize;
                const float rz = m_seedSystem->Value01(seed + 37U);
                const float cellNoise = m_seedSystem->FractalNoise2D(
                    ((chunk.origin.x + rx) * 0.0012f) + chunk.morphPhase,
                    ((chunk.origin.y + ry) * 0.0012f) - chunk.morphPhase,
                    seed,
                    3);
                const float cellular = 1.0f - m_seedSystem->Worley2D(
                    ((chunk.origin.x + rx) * 0.0018f) + chunk.morphPhase,
                    ((chunk.origin.y + ry) * 0.0018f) + chunk.morphPhase,
                    seed);

                const float structure = Saturate((cellNoise * 0.72f) + (cellular * 0.42f) - (1.0f - chunk.coverage) * 0.52f);
                if (structure <= 0.035f)
                {
                    continue;
                }

                const float3 wind = Scale(material.windDirection, windOffset);
                const float radius = (230.0f + (m_seedSystem->Value01(seed + 53U) * 420.0f)) * lod.radiusScale * Lerp(0.65f, 1.35f, chunk.coverage);
                const float height = material.cloudBaseHeight + (rz * verticalSpan);

                outInstances[outCount] = CloudInstanceCPU
                {
                    Add(Add(chunk.origin, wind), float3{ rx, ry, height - chunk.origin.z }),
                    radius,
                    Saturate(chunk.density * structure * lod.alphaScale),
                    chunk.coverage,
                    material.turbulence,
                    static_cast<float>(seed & 0xFFFFU) * (1.0f / 65535.0f),
                    material.ambientTint,
                    material.lightDimming
                };

                ++outCount;
            }
        }
    }

    int CloudVolumeGrid::GetActiveChunkCount() const
    {
        return m_activeChunkCount;
    }

    CloudChunk& CloudVolumeGrid::GetOrCreateChunk(const int gridX, const int gridY, const CloudMaterialState& material)
    {
        for (int index = 0; index < kMaxCloudChunks; ++index)
        {
            if (m_chunks[index].active && m_chunks[index].gridX == gridX && m_chunks[index].gridY == gridY)
            {
                return m_chunks[index];
            }
        }

        int target = 0;
        for (int index = 0; index < kMaxCloudChunks; ++index)
        {
            if (!m_chunks[index].active)
            {
                target = index;
                break;
            }
        }

        PopulateChunk(m_chunks[target], gridX, gridY, material);
        return m_chunks[target];
    }

    void CloudVolumeGrid::PopulateChunk(CloudChunk& chunk, const int gridX, const int gridY, const CloudMaterialState& material)
    {
        chunk.gridX = gridX;
        chunk.gridY = gridY;
        chunk.seed = m_seedSystem != nullptr ? m_seedSystem->MakeRegionSeed(gridX, gridY) : 0U;
        chunk.origin = float3
        {
            (static_cast<float>(gridX) + 0.5f) * m_config.chunkSize,
            (static_cast<float>(gridY) + 0.5f) * m_config.chunkSize,
            material.cloudBaseHeight
        };
        chunk.coverage = material.coverage;
        chunk.density = material.density;
        chunk.morphPhase = m_seedSystem != nullptr ? m_seedSystem->Value01(chunk.seed) * 64.0f : 0.0f;
        chunk.active = true;
    }
}
