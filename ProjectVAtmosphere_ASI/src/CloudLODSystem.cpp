#include "CloudLODSystem.h"

namespace PVA
{
    void CloudLODSystem::Initialize(const float drawDistance)
    {
        m_drawDistance = drawDistance;
    }

    CloudLODResult CloudLODSystem::Evaluate(const float distanceToCamera, const float coverage, const float density) const
    {
        if (distanceToCamera > m_drawDistance || coverage <= 0.015f || density <= 0.015f)
        {
            return CloudLODResult{ 0, 1.0f, 0.0f, false };
        }

        const float distance01 = Saturate(distanceToCamera / m_drawDistance);
        const float weatherComplexity = Saturate((coverage * 0.72f) + (density * 0.28f));

        int instanceCount = 2;
        if (distance01 < 0.35f)
        {
            instanceCount = 8;
        }
        else if (distance01 < 0.60f)
        {
            instanceCount = 5;
        }
        else if (distance01 < 0.82f)
        {
            instanceCount = 3;
        }

        instanceCount = static_cast<int>(static_cast<float>(instanceCount) * Lerp(0.55f, 1.35f, weatherComplexity));
        if (instanceCount < 1)
        {
            instanceCount = 1;
        }

        const float fade = 1.0f - SmoothStep(0.82f, 1.0f, distance01);
        return CloudLODResult
        {
            instanceCount,
            Lerp(1.2f, 2.4f, distance01),
            fade,
            true
        };
    }
}
