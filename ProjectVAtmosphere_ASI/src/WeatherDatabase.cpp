#include "WeatherDatabase.h"
#include <cstdio>

namespace
{
    // Reusable buffer for diagnostic strings (thread-unsafe, debug only)
    constexpr int kDiagnosticBufferSize = 512;
    thread_local char g_diagnosticBuffer[kDiagnosticBufferSize] = {};

    // Preset list string (compile-time constructed)
    constexpr const char* kPresetListString =
        "Clear | "
        "Few Clouds | "
        "Scattered Clouds | "
        "Broken Clouds | "
        "Overcast | "
        "Storm | "
        "Thunderstorm";
}

const char* WeatherDatabase::GetPresetList()
{
    return kPresetListString;
}

const char* WeatherDatabase::GetPresetDiagnostics(const WeatherState state)
{
    if (!IsValidState(static_cast<int>(state)))
    {
        return "Invalid weather state";
    }

    const WeatherPreset& preset = GetWeatherPreset(state);
    const char* name = GetWeatherStateName(state);

    // Format preset data into buffer
    snprintf(
        g_diagnosticBuffer,
        kDiagnosticBufferSize,
        "%s:\n"
        "  Cloud Coverage: %.2f | Cloud Density: %.2f\n"
        "  Wind Speed: %.2f m/s | Turbulence: %.2f\n"
        "  Storm Intensity: %.2f | Shadow Strength: %.2f\n"
        "  Light Dimming: %.2f\n"
        "  Ambient Tint: [%.3f, %.3f, %.3f]\n"
        "  Cloud Height: %.0f - %.0f m",
        name,
        preset.cloudCoverage,
        preset.cloudDensity,
        preset.windSpeed,
        preset.turbulence,
        preset.stormIntensity,
        preset.shadowStrength,
        preset.lightDimming,
        preset.ambientTint.x,
        preset.ambientTint.y,
        preset.ambientTint.z,
        preset.cloudBaseHeight,
        preset.cloudTopHeight);

    return g_diagnosticBuffer;
}
