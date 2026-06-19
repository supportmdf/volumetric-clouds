#pragma once

#include "MathUtils.h"

enum class WeatherState : int
{
    Clear = 0,
    FewClouds,
    ScatteredClouds,
    BrokenClouds,
    Overcast,
    Storm,
    Thunderstorm,
    Count
};

struct WeatherPreset
{
    float cloudCoverage;
    float cloudDensity;
    float windSpeed;
    float turbulence;
    float stormIntensity;
    float shadowStrength;
    float3 ambientTint;
    float cloudBaseHeight;
    float cloudTopHeight;
    float lightDimming;
};

const WeatherPreset& GetWeatherPreset(WeatherState state);
const WeatherPreset& InterpolateWeatherPresets(const WeatherPreset& fromPreset, const WeatherPreset& toPreset, float factor, WeatherPreset& outPreset);
const char* GetWeatherStateName(WeatherState state);
const char* GetNativeWeatherName(WeatherState state);
unsigned int GetNativeWeatherHash(WeatherState state);
WeatherState GetNextAutomaticWeatherState(WeatherState state);
bool IsValidWeatherState(int state);
