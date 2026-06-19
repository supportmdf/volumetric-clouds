#pragma once

#include "MathUtils.h"
#include "WeatherPresets.h"

namespace PVA
{
    constexpr int kCloudGridRadius = 5;
    constexpr int kCloudGridDiameter = (kCloudGridRadius * 2) + 1;
    constexpr int kMaxCloudChunks = kCloudGridDiameter * kCloudGridDiameter;
    constexpr int kMaxCloudInstances = 768;

    struct CloudRuntimeConfig
    {
        bool enabled;
        bool suppressVanillaCloudHats;
        bool depthFriendlyMode;
        bool cloudShadowOverlay;
        unsigned int worldSeed;
        float chunkSize;
        float drawDistance;
        float nearFadeDistance;
        float horizonFadeStart;
        float horizonFadeEnd;
        int maxInstances;
    };

    struct CloudCameraState
    {
        float3 position;
        float3 rotationDegrees;
        float3 forward;
        float3 right;
        float3 up;
        float4x4 viewProjection;
        float fovDegrees;
        float aspectRatio;
        bool valid;
    };

    struct CloudMaterialState
    {
        float coverage;
        float density;
        float turbulence;
        float stormIntensity;
        float shadowStrength;
        float lightDimming;
        float cloudBaseHeight;
        float cloudTopHeight;
        float3 ambientTint;
        float3 windDirection;
        float windSpeed;
        float timeSeconds;
    };

    struct CloudChunk
    {
        int gridX;
        int gridY;
        unsigned int seed;
        float3 origin;
        float coverage;
        float density;
        float morphPhase;
        bool active;
    };

    struct CloudInstanceCPU
    {
        float3 center;
        float radius;
        float density;
        float coverage;
        float turbulence;
        float seed;
        float3 tint;
        float lightDimming;
    };

    struct CloudFrameContext
    {
        CloudCameraState camera;
        CloudMaterialState material;
        CloudInstanceCPU instances[kMaxCloudInstances];
        int instanceCount;
        int activeChunkCount;
        bool rendererEnabled;
    };

    struct GlobalCloudCoreState
    {
        int enabled;
        int initialized;
        int activeChunks;
        int visibleInstances;
        float coverage;
        float density;
        float windX;
        float windY;
        float windZ;
    };

    inline CloudRuntimeConfig DefaultCloudRuntimeConfig()
    {
        return CloudRuntimeConfig
        {
            true,
            true,
            true,
            false,
            0x50564143U,
            1150.0f,
            8500.0f,
            450.0f,
            6400.0f,
            8800.0f,
            kMaxCloudInstances
        };
    }
}
