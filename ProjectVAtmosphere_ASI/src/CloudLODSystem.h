#pragma once

#include "CloudTypes.h"

namespace PVA
{
    struct CloudLODResult
    {
        int instanceCount;
        float radiusScale;
        float alphaScale;
        bool visible;
    };

    class CloudLODSystem
    {
    public:
        void Initialize(float drawDistance);
        CloudLODResult Evaluate(float distanceToCamera, float coverage, float density) const;

    private:
        float m_drawDistance = 8500.0f;
    };
}
