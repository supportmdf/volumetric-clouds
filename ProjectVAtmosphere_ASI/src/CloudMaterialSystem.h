#pragma once

#include "CloudTypes.h"

namespace PVA
{
    class CloudMaterialSystem
    {
    public:
        void Initialize();
        void UpdateFromWeather(const WeatherPreset& preset, float elapsedSeconds);

        const CloudMaterialState& GetState() const;

    private:
        CloudMaterialState m_state = {};
    };
}
