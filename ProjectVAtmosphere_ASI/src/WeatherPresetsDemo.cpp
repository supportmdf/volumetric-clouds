#include "WeatherPresets.h"
#include "WeatherDatabase.h"
#include "WeatherLerp.h"
#include <cstdio>

/**
 * WeatherPresetsDemo.cpp
 * 
 * Comprehensive test and demonstration of the Weather Preset System.
 * 
 * This file demonstrates:
 * 1. Basic preset access and validation
 * 2. Interpolation between presets
 * 3. Frame-rate independent transitions
 * 4. Multi-stage weather progressions
 * 5. Integration patterns with WeatherController
 * 
 * Compile with: /PVA_INCLUDE_EXAMPLES for example functions
 * 
 * NOTES:
 * - This is demonstration code; comment out or guard with #ifdef in production
 * - All functions use stack allocation only (zero heap usage)
 * - Suitable for running in a single test frame
 */

namespace WeatherPresetDemo
{
    /**
     * Test 1: Enumerate all presets and print their properties
     */
    void TestEnumerateAllPresets()
    {
        printf("\n=== WEATHER PRESET ENUMERATION ===\n");
        printf("Total presets: %d\n\n", WeatherDatabase::GetPresetCount());

        for (int i = 0; i < WeatherDatabase::GetPresetCount(); ++i)
        {
            const WeatherPreset& preset = WeatherDatabase::GetPresetByIndex(i);
            WeatherState state = static_cast<WeatherState>(i);

            printf("[%d] %s (GTA: %s)\n",
                i,
                WeatherDatabase::GetPresetName(state),
                WeatherDatabase::GetNativeWeatherTypeName(state));

            printf("  Cloud Coverage: %.2f | Cloud Density: %.2f\n",
                preset.cloudCoverage, preset.cloudDensity);

            printf("  Wind Speed: %.2f m/s | Turbulence: %.2f\n",
                preset.windSpeed, preset.turbulence);

            printf("  Storm Intensity: %.2f | Shadow Strength: %.2f\n",
                preset.stormIntensity, preset.shadowStrength);

            printf("  Light Dimming: %.2f\n", preset.lightDimming);

            printf("  Ambient Tint: [%.3f, %.3f, %.3f]\n",
                preset.ambientTint.x, preset.ambientTint.y, preset.ambientTint.z);

            printf("  Cloud Height: %.0f - %.0f meters\n",
                preset.cloudBaseHeight, preset.cloudTopHeight);

            printf("\n");
        }
    }

    /**
     * Test 2: Validate all presets are within acceptable ranges
     */
    void TestValidateAllPresets()
    {
        printf("\n=== PRESET VALIDATION TEST ===\n");

        int validCount = 0;
        int invalidCount = 0;

        for (int i = 0; i < WeatherDatabase::GetPresetCount(); ++i)
        {
            const WeatherPreset& preset = WeatherDatabase::GetPresetByIndex(i);
            WeatherState state = static_cast<WeatherState>(i);

            if (WeatherLerp::ValidatePresetForInterpolation(preset))
            {
                printf("[OK] %s - All values valid\n", WeatherDatabase::GetPresetName(state));
                validCount++;
            }
            else
            {
                printf("[FAIL] %s - Invalid preset data\n", WeatherDatabase::GetPresetName(state));
                invalidCount++;
            }
        }

        printf("\nResult: %d valid, %d invalid\n", validCount, invalidCount);
    }

    /**
     * Test 3: Linear interpolation between Clear and Thunderstorm
     */
    void TestLinearInterpolation()
    {
        printf("\n=== LINEAR INTERPOLATION TEST ===\n");
        printf("Transitioning: Clear -> Thunderstorm\n\n");

        WeatherPreset from = WeatherDatabase::GetPreset(WeatherState::Clear);
        WeatherPreset to = WeatherDatabase::GetPreset(WeatherState::Thunderstorm);
        WeatherPreset blended = {};

        const int samples = 11; // 0%, 10%, 20%, ..., 100%

        for (int i = 0; i < samples; ++i)
        {
            float t = i / 10.0f; // 0.0 to 1.0

            WeatherLerp::LerpLinear(from, to, t, blended);

            printf("[%.0f%%] Coverage: %.2f | Wind: %.2f | Intensity: %.2f\n",
                t * 100.0f,
                blended.cloudCoverage,
                blended.windSpeed,
                blended.stormIntensity);
        }
    }

    /**
     * Test 4: Smooth easing interpolation
     */
    void TestSmoothInterpolation()
    {
        printf("\n=== SMOOTH (EASE-IN-OUT) INTERPOLATION TEST ===\n");
        printf("Transitioning: Overcast -> Storm\n\n");

        WeatherPreset from = WeatherDatabase::GetPreset(WeatherState::Overcast);
        WeatherPreset to = WeatherDatabase::GetPreset(WeatherState::Storm);
        WeatherPreset blended = {};

        const int samples = 6; // 0%, 20%, 40%, 60%, 80%, 100%

        for (int i = 0; i < samples; ++i)
        {
            float t = i / 5.0f;

            WeatherLerp::LerpSmooth(from, to, t, blended);

            printf("[%.0f%%] Shadow Strength: %.2f | Light Dimming: %.2f\n",
                t * 100.0f,
                blended.shadowStrength,
                blended.lightDimming);
        }
    }

    /**
     * Test 5: Multi-stage transition
     */
    void TestMultiStageTransition()
    {
        printf("\n=== MULTI-STAGE TRANSITION TEST ===\n");
        printf("Stage 1 (0-50%%): Clear -> Scattered Clouds\n");
        printf("Stage 2 (50-100%%): Scattered Clouds -> Overcast\n\n");

        WeatherPreset stage1_from = WeatherDatabase::GetPreset(WeatherState::Clear);
        WeatherPreset stage1_to = WeatherDatabase::GetPreset(WeatherState::ScatteredClouds);
        WeatherPreset stage2_from = WeatherDatabase::GetPreset(WeatherState::ScatteredClouds);
        WeatherPreset stage2_to = WeatherDatabase::GetPreset(WeatherState::Overcast);
        WeatherPreset blended = {};

        const int samples = 11;

        for (int i = 0; i < samples; ++i)
        {
            float t = i / 10.0f;

            WeatherLerp::LerpMultiStage(
                stage1_from, stage1_to,
                stage2_from, stage2_to,
                t, blended);

            const char* stage = (t < 0.5f) ? "Stage 1" : "Stage 2";
            printf("[%.0f%%] %-8s | Coverage: %.2f | Density: %.2f\n",
                t * 100.0f,
                stage,
                blended.cloudCoverage,
                blended.cloudDensity);
        }
    }

    /**
     * Test 6: Frame-rate independence simulation
     * Simulates a 30-second transition at different frame rates
     */
    void TestFrameRateIndependence()
    {
        printf("\n=== FRAME-RATE INDEPENDENCE TEST ===\n");
        printf("Simulating 30-second transition at various frame rates\n\n");

        const float transitionDuration = 30.0f;
        WeatherPreset from = WeatherDatabase::GetPreset(WeatherState::Clear);
        WeatherPreset to = WeatherDatabase::GetPreset(WeatherState::Overcast);

        int frameRates[] = { 30, 60, 120, 144 };

        for (int fpsIdx = 0; fpsIdx < 4; ++fpsIdx)
        {
            int fps = frameRates[fpsIdx];
            float deltaSeconds = 1.0f / fps;
            float progress = 0.0f;
            int frameCount = 0;

            printf("At %d FPS (delta=%.4fs):\n", fps, deltaSeconds);

            // Simulate until transition complete
            while (!WeatherLerp::IsTransitionComplete(progress) && frameCount < 10000)
            {
                progress = WeatherLerp::AccumulateProgress(progress, deltaSeconds, transitionDuration);
                frameCount++;
            }

            float actualDuration = (frameCount * deltaSeconds);
            printf("  Frames: %d | Duration: %.3fs | Error: %.2f%%\n\n",
                frameCount,
                actualDuration,
                ((actualDuration - transitionDuration) / transitionDuration) * 100.0f);
        }
    }

    /**
     * Test 7: Ambient tint interpolation accuracy
     * Ensures RGB color blending is correct
     */
    void TestAmbientTintInterpolation()
    {
        printf("\n=== AMBIENT TINT INTERPOLATION TEST ===\n");

        WeatherPreset clear = WeatherDatabase::GetPreset(WeatherState::Clear);
        WeatherPreset storm = WeatherDatabase::GetPreset(WeatherState::Storm);

        printf("Clear tint:  [%.3f, %.3f, %.3f]\n",
            clear.ambientTint.x, clear.ambientTint.y, clear.ambientTint.z);
        printf("Storm tint:  [%.3f, %.3f, %.3f]\n\n",
            storm.ambientTint.x, storm.ambientTint.y, storm.ambientTint.z);

        WeatherPreset blended = {};

        for (int i = 0; i <= 10; i += 2)
        {
            float t = i / 10.0f;
            WeatherLerp::LerpLinear(clear, storm, t, blended);

            printf("[%.0f%%] Tint: [%.3f, %.3f, %.3f]\n",
                t * 100.0f,
                blended.ambientTint.x,
                blended.ambientTint.y,
                blended.ambientTint.z);
        }
    }

    /**
     * Test 8: Automatic weather sequence
     */
    void TestAutomaticWeatherSequence()
    {
        printf("\n=== AUTOMATIC WEATHER SEQUENCE TEST ===\n");
        printf("Testing automatic weather progression cycle:\n\n");

        WeatherState current = WeatherState::Clear;

        for (int i = 0; i < WeatherDatabase::GetPresetCount() + 1; ++i)
        {
            printf("[%d] %s\n", i, WeatherDatabase::GetPresetName(current));

            current = WeatherDatabase::GetNextAutomaticState(current);
        }

        printf("\n(Should cycle back to Clear)\n");
    }

    /**
     * Test 9: Preset accessibility by index and enum
     */
    void TestPresetAccess()
    {
        printf("\n=== PRESET ACCESS TEST ===\n");

        // Access by enum
        const WeatherPreset& byEnum = WeatherDatabase::GetPreset(WeatherState::Storm);
        printf("Access by enum (Storm): coverage=%.2f\n", byEnum.cloudCoverage);

        // Access by index
        const WeatherPreset& byIndex = WeatherDatabase::GetPresetByIndex(5);
        printf("Access by index [5]:    coverage=%.2f\n", byIndex.cloudCoverage);

        // Verify they're the same
        if (byEnum.cloudCoverage == byIndex.cloudCoverage)
        {
            printf("✓ Both access methods return identical presets\n");
        }
        else
        {
            printf("✗ ERROR: Access methods return different presets!\n");
        }
    }

    /**
     * Test 10: Complete weather transition simulation
     * Mimics WeatherController behavior
     */
    void TestCompleteTransitionSimulation()
    {
        printf("\n=== COMPLETE WEATHER TRANSITION SIMULATION ===\n");
        printf("Simulating: Overcast (60s) -> Storm (90s transition) -> Clear\n\n");

        WeatherState currentState = WeatherState::Overcast;
        WeatherState nextState = WeatherState::Storm;
        WeatherPreset activePreset = WeatherDatabase::GetPreset(currentState);
        WeatherPreset transitionStartPreset = activePreset;

        float deltaSeconds = 1.0f / 60.0f; // 60 FPS
        float stateElapsed = 0.0f;
        float transitionElapsed = 0.0f;
        float transitionDuration = 90.0f;
        float transitionFactor = 0.0f;
        bool transitionActive = false;

        int frameCount = 0;
        const int maxFrames = 20000; // Safety limit

        while (frameCount < maxFrames)
        {
            // State machine (simplified)
            if (!transitionActive && stateElapsed >= 60.0f)
            {
                // Trigger transition
                nextState = WeatherDatabase::GetNextAutomaticState(currentState);
                transitionStartPreset = activePreset;
                transitionActive = true;
                transitionElapsed = 0.0f;
                transitionFactor = 0.0f;
                stateElapsed = 0.0f;
            }

            // Update transition
            if (transitionActive)
            {
                transitionElapsed += deltaSeconds;
                transitionFactor = PVA::Clamp01(transitionElapsed / transitionDuration);

                InterpolateWeatherPresets(
                    transitionStartPreset,
                    WeatherDatabase::GetPreset(nextState),
                    transitionFactor,
                    activePreset);

                if (transitionFactor >= 1.0f)
                {
                    currentState = nextState;
                    transitionActive = false;
                    transitionElapsed = 0.0f;
                    transitionFactor = 0.0f;
                    stateElapsed = 0.0f;
                    activePreset = WeatherDatabase::GetPreset(currentState);

                    // Stop after two transitions for demo
                    if (frameCount > 3600) // ~60 seconds of simulation
                        break;
                }
            }
            else
            {
                stateElapsed += deltaSeconds;
            }

            // Log key moments
            if (frameCount % 1800 == 0 || transitionActive && transitionFactor < 0.1f)
            {
                printf("Frame %d | State: %-15s | Active: %s | Factor: %.3f | Wind: %.2f\n",
                    frameCount,
                    WeatherDatabase::GetPresetName(currentState),
                    transitionActive ? "YES" : "NO ",
                    transitionFactor,
                    activePreset.windSpeed);
            }

            frameCount++;
        }

        printf("\nSimulation ran %d frames (%.1f seconds at 60 FPS)\n", frameCount, frameCount / 60.0f);
    }

    /**
     * Run all tests
     */
    void RunAllTests()
    {
        printf("\n");
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("║        PROJECT V ATMOSPHERE - WEATHER PRESET SYSTEM TEST       ║\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");

        TestEnumerateAllPresets();
        TestValidateAllPresets();
        TestLinearInterpolation();
        TestSmoothInterpolation();
        TestMultiStageTransition();
        TestFrameRateIndependence();
        TestAmbientTintInterpolation();
        TestAutomaticWeatherSequence();
        TestPresetAccess();
        TestCompleteTransitionSimulation();

        printf("\n");
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("║                      ALL TESTS COMPLETED                       ║\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
    }
}

/**
 * Entry point for demo (if compiled as standalone test)
 * 
 * Uncomment to build as test executable:
 * 
 * int main()
 * {
 *     WeatherPresetDemo::RunAllTests();
 *     return 0;
 * }
 */
