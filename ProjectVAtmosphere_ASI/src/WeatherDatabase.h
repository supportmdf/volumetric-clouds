#pragma once

#include "WeatherPresets.h"

/**
 * WeatherDatabase
 * 
 * Provides a unified interface for accessing, validating, and working with
 * the complete weather preset database. Acts as the data backbone for the
 * entire atmospheric system.
 * 
 * Design:
 * - Zero-allocation (all data is static const)
 * - Cache-friendly (linear array storage)
 * - Frame-rate independent
 * - No external dependencies (only MathUtils)
 */

namespace WeatherDatabase
{
    /**
     * Retrieve the number of available weather presets
     * @return Total weather preset count (7 presets)
     */
    constexpr int GetPresetCount()
    {
        return static_cast<int>(WeatherState::Count);
    }

    /**
     * Check if a weather preset index is valid
     * @param presetIndex The index to validate
     * @return true if index is in valid range [0, Count)
     */
    inline bool IsValidPresetIndex(int presetIndex)
    {
        return presetIndex >= 0 && presetIndex < GetPresetCount();
    }

    /**
     * Get preset from WeatherState enum
     * @param state The weather state
     * @return Reference to the immutable weather preset
     */
    inline const WeatherPreset& GetPreset(WeatherState state)
    {
        return GetWeatherPreset(state);
    }

    /**
     * Get preset from numeric index
     * @param index The preset index (0-6)
     * @return Reference to the immutable weather preset, or Clear if invalid
     */
    inline const WeatherPreset& GetPresetByIndex(int index)
    {
        if (!IsValidPresetIndex(index))
        {
            return GetWeatherPreset(WeatherState::Clear);
        }
        return GetWeatherPreset(static_cast<WeatherState>(index));
    }

    /**
     * Get human-readable name for a weather state
     * @param state The weather state
     * @return C-string name (e.g., "Clear", "Thunderstorm")
     */
    inline const char* GetPresetName(WeatherState state)
    {
        return GetWeatherStateName(state);
    }

    /**
     * Get the GTA V native weather type name for a state
     * @param state The weather state
     * @return C-string GTA native name (e.g., "CLEAR", "THUNDER")
     */
    inline const char* GetNativeWeatherTypeName(WeatherState state)
    {
        return GetNativeWeatherName(state);
    }

    /**
     * Get the GTA V native weather type hash for a state
     * @param state The weather state
     * @return JOAAT hash for native API calls
     */
    inline unsigned int GetNativeWeatherTypeHash(WeatherState state)
    {
        return GetNativeWeatherHash(state);
    }

    /**
     * Perform smooth interpolation between two weather presets
     * @param fromPreset Starting weather preset
     * @param toPreset Ending weather preset
     * @param factor Interpolation factor [0.0, 1.0]; clamped internally
     * @param outPreset Output preset (will be modified with interpolated values)
     * @return Reference to outPreset for chaining
     */
    inline const WeatherPreset& InterpolatePresets(
        const WeatherPreset& fromPreset,
        const WeatherPreset& toPreset,
        float factor,
        WeatherPreset& outPreset)
    {
        return InterpolateWeatherPresets(fromPreset, toPreset, factor, outPreset);
    }

    /**
     * Blend between two weather states with automatic preset lookup
     * @param currentState Current weather state
     * @param nextState Target weather state
     * @param factor Blend factor [0.0, 1.0]; clamped internally
     * @param outPreset Output preset (will be modified)
     * @return Reference to outPreset for chaining
     */
    inline const WeatherPreset& BlendWeatherStates(
        WeatherState currentState,
        WeatherState nextState,
        float factor,
        WeatherPreset& outPreset)
    {
        return InterpolateWeatherPresets(
            GetWeatherPreset(currentState),
            GetWeatherPreset(nextState),
            factor,
            outPreset);
    }

    /**
     * Get the next weather state in the automatic sequence
     * Used by auto-weather cycling system
     * @param currentState The current weather state
     * @return The next state in the automatic progression
     */
    inline WeatherState GetNextAutomaticState(WeatherState currentState)
    {
        return GetNextAutomaticWeatherState(currentState);
    }

    /**
     * Validate a weather state value
     * @param stateValue The value to validate (typically cast from int)
     * @return true if valid and representable as WeatherState
     */
    inline bool IsValidState(int stateValue)
    {
        return IsValidWeatherState(stateValue);
    }

    /**
     * Get all weather preset information as a single string
     * Useful for debugging and logging
     * @return Human-readable string with all preset names
     * 
     * Example output:
     * "Clear | Few Clouds | Scattered Clouds | Broken Clouds | 
     *  Overcast | Storm | Thunderstorm"
     */
    const char* GetPresetList();

    /**
     * Get diagnostic info for a specific preset
     * @param state The weather state
     * @return Formatted string with preset properties (for debugging)
     */
    const char* GetPresetDiagnostics(WeatherState state);
}
