#include "WeatherPresets.h"

namespace
{
    constexpr unsigned int ToLowerAscii(const unsigned int value)
    {
        return (value >= 'A' && value <= 'Z') ? (value + 32U) : value;
    }

    constexpr unsigned int JoaatStep(const unsigned int hash, const char value)
    {
        unsigned int result = hash + ToLowerAscii(static_cast<unsigned int>(value));
        result += (result << 10U);
        result ^= (result >> 6U);
        return result;
    }

    constexpr unsigned int JoaatFinalize(const unsigned int hash)
    {
        unsigned int result = hash;
        result += (result << 3U);
        result ^= (result >> 11U);
        result += (result << 15U);
        return result;
    }

    constexpr unsigned int Joaat(const char* text, const unsigned int hash = 0U)
    {
        return (*text == '\0') ? JoaatFinalize(hash) : Joaat(text + 1, JoaatStep(hash, *text));
    }

    constexpr WeatherPreset kWeatherPresets[] =
    {
        // Clear
        {
            0.05f,
            0.08f,
            1.20f,
            0.03f,
            0.00f,
            0.18f,
            { 1.00f, 0.97f, 0.90f },
            1450.0f,
            3100.0f,
            0.00f
        },

        // Few Clouds
        {
            0.18f,
            0.22f,
            1.80f,
            0.08f,
            0.00f,
            0.26f,
            { 0.96f, 0.97f, 0.94f },
            1350.0f,
            3400.0f,
            0.05f
        },

        // Scattered Clouds
        {
            0.38f,
            0.42f,
            2.40f,
            0.16f,
            0.00f,
            0.38f,
            { 0.88f, 0.92f, 0.96f },
            1250.0f,
            3900.0f,
            0.14f
        },

        // Broken Clouds
        {
            0.62f,
            0.60f,
            3.10f,
            0.28f,
            0.03f,
            0.56f,
            { 0.76f, 0.82f, 0.90f },
            1100.0f,
            4300.0f,
            0.28f
        },

        // Overcast
        {
            0.86f,
            0.78f,
            4.00f,
            0.40f,
            0.10f,
            0.74f,
            { 0.62f, 0.68f, 0.75f },
            950.0f,
            4700.0f,
            0.48f
        },

        // Storm
        {
            0.96f,
            0.92f,
            7.50f,
            0.72f,
            0.68f,
            0.88f,
            { 0.44f, 0.50f, 0.58f },
            650.0f,
            5600.0f,
            0.70f
        },

        // Thunderstorm
        {
            1.00f,
            1.00f,
            10.50f,
            0.94f,
            1.00f,
            0.96f,
            { 0.34f, 0.38f, 0.47f },
            450.0f,
            6200.0f,
            0.86f
        }
    };

    constexpr const char* kWeatherStateNames[] =
    {
        "Clear",
        "Few Clouds",
        "Scattered Clouds",
        "Broken Clouds",
        "Overcast",
        "Storm",
        "Thunderstorm"
    };

    constexpr const char* kNativeWeatherNames[] =
    {
        "CLEAR",
        "CLOUDS",
        "SMOG",
        "OVERCAST",
        "OVERCAST",
        "RAIN",
        "THUNDER"
    };

    constexpr unsigned int kNativeWeatherHashes[] =
    {
        Joaat("CLEAR"),
        Joaat("CLOUDS"),
        Joaat("SMOG"),
        Joaat("OVERCAST"),
        Joaat("OVERCAST"),
        Joaat("RAIN"),
        Joaat("THUNDER")
    };

    constexpr WeatherState kAutomaticSequence[] =
    {
        WeatherState::Clear,
        WeatherState::FewClouds,
        WeatherState::ScatteredClouds,
        WeatherState::BrokenClouds,
        WeatherState::Overcast,
        WeatherState::Storm,
        WeatherState::Thunderstorm
    };

    constexpr int ToIndex(const WeatherState state)
    {
        return static_cast<int>(state);
    }
}

const WeatherPreset& GetWeatherPreset(const WeatherState state)
{
    if (!IsValidWeatherState(ToIndex(state)))
    {
        return kWeatherPresets[ToIndex(WeatherState::Clear)];
    }

    return kWeatherPresets[ToIndex(state)];
}

const WeatherPreset& InterpolateWeatherPresets(const WeatherPreset& fromPreset, const WeatherPreset& toPreset, const float factor, WeatherPreset& outPreset)
{
    const float t = PVA::Clamp01(factor);

    outPreset.cloudCoverage = PVA::Lerp(fromPreset.cloudCoverage, toPreset.cloudCoverage, t);
    outPreset.cloudDensity = PVA::Lerp(fromPreset.cloudDensity, toPreset.cloudDensity, t);
    outPreset.windSpeed = PVA::Lerp(fromPreset.windSpeed, toPreset.windSpeed, t);
    outPreset.turbulence = PVA::Lerp(fromPreset.turbulence, toPreset.turbulence, t);
    outPreset.stormIntensity = PVA::Lerp(fromPreset.stormIntensity, toPreset.stormIntensity, t);
    outPreset.shadowStrength = PVA::Lerp(fromPreset.shadowStrength, toPreset.shadowStrength, t);
    outPreset.ambientTint = PVA::Lerp(fromPreset.ambientTint, toPreset.ambientTint, t);
    outPreset.cloudBaseHeight = PVA::Lerp(fromPreset.cloudBaseHeight, toPreset.cloudBaseHeight, t);
    outPreset.cloudTopHeight = PVA::Lerp(fromPreset.cloudTopHeight, toPreset.cloudTopHeight, t);
    outPreset.lightDimming = PVA::Lerp(fromPreset.lightDimming, toPreset.lightDimming, t);

    return outPreset;
}

const char* GetWeatherStateName(const WeatherState state)
{
    if (!IsValidWeatherState(ToIndex(state)))
    {
        return kWeatherStateNames[ToIndex(WeatherState::Clear)];
    }

    return kWeatherStateNames[ToIndex(state)];
}

const char* GetNativeWeatherName(const WeatherState state)
{
    if (!IsValidWeatherState(ToIndex(state)))
    {
        return kNativeWeatherNames[ToIndex(WeatherState::Clear)];
    }

    return kNativeWeatherNames[ToIndex(state)];
}

unsigned int GetNativeWeatherHash(const WeatherState state)
{
    if (!IsValidWeatherState(ToIndex(state)))
    {
        return kNativeWeatherHashes[ToIndex(WeatherState::Clear)];
    }

    return kNativeWeatherHashes[ToIndex(state)];
}

WeatherState GetNextAutomaticWeatherState(const WeatherState state)
{
    constexpr int sequenceCount = static_cast<int>(sizeof(kAutomaticSequence) / sizeof(kAutomaticSequence[0]));

    for (int index = 0; index < sequenceCount; ++index)
    {
        if (kAutomaticSequence[index] == state)
        {
            const int nextIndex = (index + 1) % sequenceCount;
            return kAutomaticSequence[nextIndex];
        }
    }

    return WeatherState::Clear;
}

bool IsValidWeatherState(const int state)
{
    return state >= 0 && state < ToIndex(WeatherState::Count);
}
