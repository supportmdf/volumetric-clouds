#pragma once

#include "CloudLODSystem.h"
#include "CloudSeedSystem.h"

namespace PVA
{
    class CloudVolumeGrid
    {
    public:
        void Initialize(const CloudRuntimeConfig& config, CloudSeedSystem* seedSystem, CloudLODSystem* lodSystem);
        void Update(const CloudCameraState& camera, const CloudMaterialState& material, float deltaSeconds);
        void BuildRenderInstances(const CloudCameraState& camera, const CloudMaterialState& material, CloudInstanceCPU* outInstances, int maxInstances, int& outCount) const;

        int GetActiveChunkCount() const;

    private:
        CloudChunk& GetOrCreateChunk(int gridX, int gridY, const CloudMaterialState& material);
        void PopulateChunk(CloudChunk& chunk, int gridX, int gridY, const CloudMaterialState& material);

        CloudChunk m_chunks[kMaxCloudChunks] = {};
        CloudRuntimeConfig m_config = {};
        CloudSeedSystem* m_seedSystem = nullptr;
        CloudLODSystem* m_lodSystem = nullptr;
        int m_activeChunkCount = 0;
        int m_refreshCursor = 0;
    };
}
