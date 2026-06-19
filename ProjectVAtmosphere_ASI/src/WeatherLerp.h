#pragma once

#include "WeatherPresets.h"

/**
 * WeatherLerp
 * 
 * Specialized interpolation utilities for smooth weather transitions.
 * Provides easing functions, multi-stage blending, and optimization helpers
 * for frame-rate independent atmospheric blending.
 * 
 * Design:
 * - All functions are inline (zero-cost abstractions)
 * - No allocations (uses stack-based output parameters)
 * - Supports both linear and eased interpolation
 * - Cache-friendly parameter passing
 */

namespace WeatherLerp
{
    /**
     * Linear interpolation factor (standard)
     * t ∈ [0, 1] → [0, 1] linearly
     * 
     * @param t Time factor [0, 1]
     * @return Easing value [0, 1]
     */
    inline float EaseLinear(float t)
    {
        return PVA::Clamp01(t);
    }

    /**
     * Ease-in-out cubic (smooth acceleration/deceleration)
     * Creates smooth starts and ends for weather transitions
     * 
     * @param t Time factor [0, 1]
     * @return Easing value [0, 1]
     */
    inline float EaseInOutCubic(float t)
    {
        t = PVA::Clamp01(t);
        if (t < 0.5f)
        {
            return 4.0f * t * t * t;
        }
        const float f = 2.0f * t - 2.0f;
        return 0.5f * f * f * f + 1.0f;
    }

    /**
     * Smooth step easing (Hermite interpolation)
     * Good for atmospheric effects
     * 
     * @param t Time factor [0, 1]
     * @return Easing value [0, 1]
     */
    inline float EaseSmoothStep(float t)
    {
        return PVA::SmoothStep(0.0f, 1.0f, t);
    }

    /**
     * Basic linear weather blend
     * Fast, suitable for most transitions
     * 
     * @param from Starting preset
     * @param to Ending preset
     * @param t Blend factor [0, 1]
     * @param out Output preset
     * @return Reference to out for chaining
     */
    inline const WeatherPreset& LerpLinear(
        const WeatherPreset& from,
        const WeatherPreset& to,
        float t,
        WeatherPreset& out)
    {
        return InterpolateWeatherPresets(from, to, EaseLinear(t), out);
    }

    /**
     * Smooth ease-in-out weather blend
     * Better visual feel for player-initiated transitions
     * 
     * @param from Starting preset
     * @param to Ending preset
     * @param t Blend factor [0, 1]
     * @param out Output preset
     * @return Reference to out for chaining
     */
    inline const WeatherPreset& LerpSmooth(
        const WeatherPreset& from,
        const WeatherPreset& to,
        float t,
        WeatherPreset& out)
    {
        return InterpolateWeatherPresets(from, to, EaseInOutCubic(t), out);
    }

    /**
     * Cubic ease-in-out weather blend
     * Maximum smoothness for cinematic transitions
     * 
     * @param from Starting preset
     * @param to Ending preset
     * @param t Blend factor [0, 1]
     * @param out Output preset
     * @return Reference to out for chaining
     */
    inline const WeatherPreset& LerpCubic(
        const WeatherPreset& from,
        const WeatherPreset& to,
        float t,
        WeatherPreset& out)
    {
        return InterpolateWeatherPresets(from, to, EaseSmoothStep(t), out);
    }

    /**
     * Multi-stage weather blend
     * Useful when transitioning through multiple intermediate states
     * 
     * Example: Clear → Few Clouds → Overcast
     * Stage 1 (t=0.0-0.5): Clear to Few Clouds
     * Stage 2 (t=0.5-1.0): Few Clouds to Overcast
     * 
     * @param stage1From Starting preset
     * @param stage1To First transition target
     * @param stage2From Second transition source (often == stage1To)
     * @param stage2To Final transition target
     * @param t Overall blend factor [0, 1]
     * @param out Output preset
     * @return Reference to out for chaining
     */
    inline const WeatherPreset& LerpMultiStage(
        const WeatherPreset& stage1From,
        const WeatherPreset& stage1To,
        const WeatherPreset& stage2From,
        const WeatherPreset& stage2To,
        float t,
        WeatherPreset& out)
    {
        t = PVA::Clamp01(t);

        if (t < 0.5f)
        {
            // First half: blend stage1
            const float stageT = t * 2.0f; // Map [0, 0.5] to [0, 1]
            return InterpolateWeatherPresets(stage1From, stage1To, EaseInOutCubic(stageT), out);
        }
        else
        {
            // Second half: blend stage2
            const float stageT = (t - 0.5f) * 2.0f; // Map [0.5, 1.0] to [0, 1]
            return InterpolateWeatherPresets(stage2From, stage2To, EaseInOutCubic(stageT), out);
        }
    }

    /**
     * Frame-rate independent delta calculation
     * Converts real-time delta into normalized transition progress
     * 
     * Example: 60 FPS, 30-second transition
     * deltaSeconds = 1/60 ≈ 0.0167
     * transitionSeconds = 30.0
     * Output: 0.0167 / 30.0 ≈ 0.000556 per frame
     * 
     * @param deltaSeconds Time elapsed this frame (from WeatherController::m_deltaSeconds)
     * @param transitionDurationSeconds Total transition time
     * @return Progress increment for this frame [0, 1]
     */
    inline float CalculateDeltaProgress(float deltaSeconds, float transitionDurationSeconds)
    {
        if (transitionDurationSeconds <= 0.001f)
        {
            return 1.0f; // Instant transition
        }
        return deltaSeconds / transitionDurationSeconds;
    }

    /**
     * Accumulate progress over multiple frames (frame-rate independent)
     * 
     * Usage pattern:
     * ```
     * static float transitionProgress = 0.0f;
     * transitionProgress = PVA::Clamp01(transitionProgress + 
     *     WeatherLerp::CalculateDeltaProgress(deltaSeconds, 60.0f));
     * 
     * WeatherPreset blended = {};
     * WeatherLerp::LerpSmooth(currentPreset, targetPreset, transitionProgress, blended);
     * ```
     * 
     * @param currentProgress Current progress [0, 1]
     * @param deltaSeconds Frame delta time (seconds)
     * @param transitionDurationSeconds Total transition duration (seconds)
     * @return New progress value, clamped [0, 1]
     */
    inline float AccumulateProgress(float currentProgress, float deltaSeconds, float transitionDurationSeconds)
    {
        return PVA::Clamp01(currentProgress + CalculateDeltaProgress(deltaSeconds, transitionDurationSeconds));
    }

    /**
     * Check if transition is effectively complete
     * Accounts for floating-point precision
     * 
     * @param progress Current transition progress
     * @param epsilon Completion threshold (default 0.999 = 99.9% done)
     * @return true if progress >= epsilon
     */
    inline bool IsTransitionComplete(float progress, float epsilon = 0.999f)
    {
        return progress >= epsilon;
    }

    /**
     * Check if transition is effectively at start
     * Accounts for floating-point precision
     * 
     * @param progress Current transition progress
     * @param epsilon Start threshold (default 0.001 = 0.1% progress)
     * @return true if progress <= epsilon
     */
    inline bool IsTransitionAtStart(float progress, float epsilon = 0.001f)
    {
        return progress <= epsilon;
    }

    /**
     * Validate weather preset for interpolation safety
     * Checks all values are within valid ranges
     * 
     * @param preset Preset to validate
     * @return true if all values are valid for interpolation
     */
    inline bool ValidatePresetForInterpolation(const WeatherPreset& preset)
    {
        // Check numeric ranges
        if (preset.cloudCoverage < 0.0f || preset.cloudCoverage > 1.0f) return false;
        if (preset.cloudDensity < 0.0f || preset.cloudDensity > 2.0f) return false;
        if (preset.windSpeed < 0.0f || preset.windSpeed > 50.0f) return false;
        if (preset.turbulence < 0.0f || preset.turbulence > 2.0f) return false;
        if (preset.stormIntensity < 0.0f || preset.stormIntensity > 1.0f) return false;
        if (preset.shadowStrength < 0.0f || preset.shadowStrength > 1.0f) return false;
        if (preset.lightDimming < 0.0f || preset.lightDimming > 1.0f) return false;

        // Check color tint is reasonable (not black/invalid)
        if (preset.ambientTint.x < 0.0f || preset.ambientTint.y < 0.0f || preset.ambientTint.z < 0.0f)
            return false;

        // Check cloud heights
        if (preset.cloudBaseHeight < 0.0f || preset.cloudTopHeight < preset.cloudBaseHeight)
            return false;

        return true;
    }
}
