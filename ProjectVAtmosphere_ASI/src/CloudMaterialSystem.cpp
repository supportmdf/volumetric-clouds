#include "CloudMaterialSystem.h"

namespace PVA
{
    void CloudMaterialSystem::Initialize()
    {
        m_state.coverage = 0.0f;
        m_state.density = 0.0f;
        m_state.turbulence = 0.0f;
        m_state.stormIntensity = 0.0f;
        m_state.shadowStrength = 0.0f;
        m_state.lightDimming = 0.0f;
        m_state.cloudBaseHeight = 1400.0f;
        m_state.cloudTopHeight = 3200.0f;
        m_state.ambientTint = float3{ 1.0f, 1.0f, 1.0f };
        m_state.windDirection = float3{ 1.0f, 0.25f, 0.0f };
        m_state.windSpeed = 0.0f;
        m_state.timeSeconds = 0.0f;
    }

    void CloudMaterialSystem::UpdateFromWeather(const WeatherPreset& preset, const float elapsedSeconds)
    {
        m_state.coverage = Saturate(preset.cloudCoverage);
        m_state.density = Saturate(preset.cloudDensity);
        m_state.turbulence = Saturate(Max(preset.turbulence, preset.stormIntensity * 0.65f));
        m_state.stormIntensity = Saturate(preset.stormIntensity);
        m_state.shadowStrength = Saturate(preset.shadowStrength);
        m_state.lightDimming = Saturate(preset.lightDimming);
        m_state.cloudBaseHeight = preset.cloudBaseHeight;
        m_state.cloudTopHeight = preset.cloudTopHeight;
        m_state.ambientTint = preset.ambientTint;

        const float directionAngle = (35.0f + (preset.stormIntensity * 28.0f)) * kDegToRad;
        m_state.windDirection = Normalize(float3{ std::cos(directionAngle), std::sin(directionAngle), 0.0f });
        m_state.windSpeed = Max(0.0f, preset.windSpeed);
        m_state.timeSeconds = elapsedSeconds;
    }

    const CloudMaterialState& CloudMaterialSystem::GetState() const
    {
        return m_state;
    }
}
