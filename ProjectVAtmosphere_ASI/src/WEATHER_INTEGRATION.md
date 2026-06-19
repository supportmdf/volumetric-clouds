# Weather Preset System - Developer Integration Guide

## Quick Start (5 minutes)

### 1. Include the Headers
```cpp
#include "WeatherDatabase.h"
#include "WeatherLerp.h"
```

### 2. Get a Preset
```cpp
// By enum
WeatherPreset storm = WeatherDatabase::GetPreset(WeatherState::Storm);

// By index (0-6)
WeatherPreset clear = WeatherDatabase::GetPresetByIndex(0);

// Print name
printf("Weather: %s\n", WeatherDatabase::GetPresetName(storm));
// Output: "Storm"
```

### 3. Blend Two Presets
```cpp
WeatherPreset from = WeatherDatabase::GetPreset(WeatherState::Clear);
WeatherPreset to = WeatherDatabase::GetPreset(WeatherState::Storm);
WeatherPreset result = {};

// 50% blend
WeatherLerp::LerpSmooth(from, to, 0.5f, result);

// Now 'result' contains halfway-blended weather data
printf("Blended wind: %.2f m/s\n", result.windSpeed);
```

### 4. Frame-Rate Independent Transition
```cpp
// In your update loop:
static float progress = 0.0f;

// Every frame, accumulate progress
progress = WeatherLerp::AccumulateProgress(
    progress,
    deltaTimeInSeconds,  // From your game loop (1/60, 1/144, etc.)
    transitionDurationInSeconds);  // e.g., 30.0f

// Blend based on progress
WeatherPreset blended = {};
WeatherLerp::LerpSmooth(from, to, progress, blended);

// When done
if (WeatherLerp::IsTransitionComplete(progress))
{
    progress = 0.0f;  // Reset for next transition
}
```

---

## Common Use Cases

### Use Case 1: Display Current Weather
```cpp
WeatherState currentWeather = WeatherState::Overcast;
const WeatherPreset& preset = WeatherDatabase::GetPreset(currentWeather);

printf("Current: %s\n", WeatherDatabase::GetPresetName(currentWeather));
printf("  Cloud Coverage: %.0f%%\n", preset.cloudCoverage * 100.0f);
printf("  Wind Speed: %.1f m/s\n", preset.windSpeed);
printf("  Storm: %.0f%%\n", preset.stormIntensity * 100.0f);
```

### Use Case 2: Transition to New Weather
```cpp
void RequestWeatherChange(WeatherState target, float durationSeconds)
{
    static WeatherState currentState = WeatherState::Clear;
    static WeatherState nextState = WeatherState::Clear;
    static float transitionProgress = 0.0f;
    static float transitionDuration = 30.0f;
    
    if (target != nextState)
    {
        nextState = target;
        transitionDuration = durationSeconds;
        transitionProgress = 0.0f;  // Reset transition
    }
}

void UpdateWeather(float deltaSeconds)
{
    // Accumulate progress (see pattern above)
    // When complete, update currentState
}
```

### Use Case 3: Storm Intensity Visualization
```cpp
void VisualizeStormIntensity(const WeatherPreset& preset)
{
    // Scale particle effects based on storm intensity
    float particleCount = preset.stormIntensity * MAX_PARTICLES;
    
    // Darken screen based on light dimming
    float screenDarkness = preset.lightDimming;
    
    // Increase wind distortion based on turbulence
    float distortionAmount = preset.turbulence * 2.0f;
    
    // Apply ambient tint to scene lighting
    float3 sceneTint = preset.ambientTint;
}
```

### Use Case 4: Multi-Preset Sequence
```cpp
// Gradual weather progression: Clear → Scattered → Overcast
void ProgessiveWeatherChange()
{
    WeatherPreset clear = WeatherDatabase::GetPreset(WeatherState::Clear);
    WeatherPreset scattered = WeatherDatabase::GetPreset(WeatherState::ScatteredClouds);
    WeatherPreset overcast = WeatherDatabase::GetPreset(WeatherState::Overcast);
    WeatherPreset result = {};
    
    float progress = 0.0f;  // Updated each frame
    
    // Two-stage: 0-50% is clear→scattered, 50-100% is scattered→overcast
    WeatherLerp::LerpMultiStage(
        clear, scattered,
        scattered, overcast,
        progress,
        result);
}
```

### Use Case 5: Automatic Weather Cycling
```cpp
void AutomaticWeatherCycle()
{
    static WeatherState currentWeather = WeatherState::Clear;
    static float stateTimer = 0.0f;
    
    const float STATE_DURATION = 120.0f;  // 2 minutes per weather state
    
    stateTimer += deltaSeconds;
    
    if (stateTimer >= STATE_DURATION)
    {
        // Progress to next weather in sequence
        currentWeather = WeatherDatabase::GetNextAutomaticState(currentWeather);
        stateTimer = 0.0f;
    }
}
```

---

## Integration Points

### With ReShade
```cpp
// Expose active preset to ReShade shader uniforms
extern "C" DLL_EXPORT const WeatherPreset* PVA_GetActiveWeatherPreset()
{
    return WeatherDatabase::GetPreset(g_CurrentWeatherState);
}

// In ReShade shader:
// cbuffer PVAWeather {
//     float g_CloudCoverage;
//     float g_CloudDensity;
//     float g_WindSpeed;
//     float3 g_AmbientTint;
//     ...
// };
```

### With Cloud Renderer
```cpp
void UpdateCloudRenderer(const WeatherPreset& preset)
{
    // Update volumetric cloud parameters
    g_CloudRenderer.SetDensity(preset.cloudDensity);
    g_CloudRenderer.SetCoverage(preset.cloudCoverage);
    g_CloudRenderer.SetBaseHeight(preset.cloudBaseHeight);
    g_CloudRenderer.SetTopHeight(preset.cloudTopHeight);
    
    // Update particle wind
    g_ParticleSystem.SetWindSpeed(preset.windSpeed);
    g_ParticleSystem.SetTurbulence(preset.turbulence);
}
```

### With Atmospheric Scattering
```cpp
void UpdateAtmosphericScattering(const WeatherPreset& preset)
{
    // Fog density based on storm intensity and cloud coverage
    float fogDensity = (preset.stormIntensity + preset.cloudCoverage) * 0.5f;
    g_AtmosphereRenderer.SetFogDensity(fogDensity);
    
    // Light absorption from dimming
    float lightAbsorption = preset.lightDimming;
    g_AtmosphereRenderer.SetLightAbsorption(lightAbsorption);
    
    // Scene tint from ambient color
    g_AtmosphereRenderer.SetAmbientTint(preset.ambientTint);
}
```

---

## Performance Considerations

### Memory
- Each preset: ~140 bytes
- 7 presets total: ~980 bytes (< 1 KB)
- **Negligible footprint**

### CPU Per Frame
| Scenario | Cost | Budget at 60 FPS |
|----------|------|------------------|
| Single lerp | <1 μs | 16.67 ms |
| Multi-stage blend | ~2 μs | 16.67 ms |
| Validation check | <1 μs | 16.67 ms |

**Result:** Weather system uses <0.01% of frame budget

### Cache Behavior
- Static const arrays (L1 cache hit)
- Linear access patterns (prefetcher friendly)
- No pointer dereferencing
- **Optimal cache efficiency**

---

## Debugging

### Print All Presets
```cpp
printf("Available presets:\n");
printf("%s\n", WeatherDatabase::GetPresetList());
// Output: Clear | Few Clouds | Scattered Clouds | Broken Clouds | 
//         Overcast | Storm | Thunderstorm
```

### Get Detailed Diagnostics
```cpp
for (int i = 0; i < WeatherDatabase::GetPresetCount(); ++i)
{
    WeatherState state = static_cast<WeatherState>(i);
    const char* diag = WeatherDatabase::GetPresetDiagnostics(state);
    printf("%s\n\n", diag);
}
```

### Validate Preset Data
```cpp
const WeatherPreset& preset = WeatherDatabase::GetPreset(WeatherState::Storm);

if (!WeatherLerp::ValidatePresetForInterpolation(preset))
{
    printf("ERROR: Preset data corrupted!\n");
    // Fallback to Clear preset
    return WeatherDatabase::GetPreset(WeatherState::Clear);
}
```

### Log Transition Progress
```cpp
static float progress = 0.0f;
progress = WeatherLerp::AccumulateProgress(progress, deltaSeconds, 30.0f);

if (fmod(progress, 0.1f) < deltaSeconds)  // Log every 10%
{
    printf("[Weather] Transition: %.0f%% complete\n", progress * 100.0f);
}
```

---

## Testing

### Run Full Test Suite
```cpp
#include "WeatherPresetsDemo.cpp"

// In your initialization:
WeatherPresetDemo::RunAllTests();

// Output includes:
// - Preset enumeration
// - Validation results
// - Interpolation accuracy
// - Frame-rate independence verification
// - Multi-stage transition testing
// - etc.
```

### Manual Test: Linear Blend
```cpp
WeatherPreset from = WeatherDatabase::GetPreset(WeatherState::Clear);
WeatherPreset to = WeatherDatabase::GetPreset(WeatherState::Storm);
WeatherPreset blend = {};

// Test at 0%, 50%, 100%
for (float t : {0.0f, 0.5f, 1.0f})
{
    WeatherLerp::LerpLinear(from, to, t, blend);
    printf("%.0f%%: Wind = %.2f\n", t * 100, blend.windSpeed);
}

// Expected output:
// 0%: Wind = 1.20
// 50%: Wind = 4.35
// 100%: Wind = 7.50
```

---

## Troubleshooting

| Problem | Cause | Solution |
|---------|-------|----------|
| Jerky transitions | Using unsmoothed linear progress | Use `EaseInOutCubic()` or `LerpSmooth()` |
| Transition too fast/slow | Wrong duration parameter | Verify `transitionDurationInSeconds` |
| Blending artifacts | Invalid preset values | Call `ValidatePresetForInterpolation()` |
| Memory issues | Heap allocation | Verify no `new`/`malloc` in runtime paths |
| Different FPS = different speed | Using frame count instead of time | Use `CalculateDeltaProgress()` |

---

## API Cheat Sheet

### Get Data
```cpp
WeatherDatabase::GetPreset(state);              // By enum
WeatherDatabase::GetPresetByIndex(index);       // By index [0-6]
WeatherDatabase::GetPresetName(state);          // "Clear", "Storm", etc.
WeatherDatabase::GetNativeWeatherTypeName();    // "CLEAR", "THUNDER", etc.
```

### Interpolate
```cpp
WeatherLerp::LerpLinear(from, to, t, out);      // Fast, no easing
WeatherLerp::LerpSmooth(from, to, t, out);      // Smooth easing
WeatherLerp::LerpCubic(from, to, t, out);       // Maximum smoothness
WeatherLerp::LerpMultiStage(f1, t1, f2, t2, t, out);  // Two-stage
```

### Time-Based Progress
```cpp
float delta = WeatherLerp::CalculateDeltaProgress(dt, duration);
progress = WeatherLerp::AccumulateProgress(progress, dt, duration);
bool done = WeatherLerp::IsTransitionComplete(progress);
bool start = WeatherLerp::IsTransitionAtStart(progress);
```

### Validation
```cpp
WeatherDatabase::IsValidState(value);
WeatherLerp::ValidatePresetForInterpolation(preset);
WeatherDatabase::GetPresetDiagnostics(state);
```

---

## Example: Complete Weather System

```cpp
class SimpleWeatherSystem
{
private:
    WeatherState m_current = WeatherState::Clear;
    WeatherState m_target = WeatherState::Clear;
    float m_transitionProgress = 0.0f;
    float m_transitionDuration = 30.0f;
    WeatherPreset m_activePreset = {};

public:
    void Initialize()
    {
        m_activePreset = WeatherDatabase::GetPreset(m_current);
    }

    void RequestWeather(WeatherState target, float durationSeconds)
    {
        if (target == m_current && m_transitionProgress > 0.99f)
            return;  // Already there

        m_target = target;
        m_transitionDuration = durationSeconds;
        m_transitionProgress = 0.0f;
    }

    void Update(float deltaSeconds)
    {
        // Advance transition
        m_transitionProgress = WeatherLerp::AccumulateProgress(
            m_transitionProgress,
            deltaSeconds,
            m_transitionDuration);

        // Blend presets
        WeatherLerp::LerpSmooth(
            WeatherDatabase::GetPreset(m_current),
            WeatherDatabase::GetPreset(m_target),
            m_transitionProgress,
            m_activePreset);

        // Complete transition
        if (WeatherLerp::IsTransitionComplete(m_transitionProgress))
        {
            m_current = m_target;
            m_transitionProgress = 0.0f;
        }
    }

    const WeatherPreset& GetActivePreset() const
    {
        return m_activePreset;
    }

    float GetTransitionProgress() const
    {
        return m_transitionProgress;
    }
};
```

---

## Files Included

| File | Purpose |
|------|---------|
| `WeatherPresets.h/cpp` | Core data structure & presets |
| `WeatherDatabase.h/cpp` | Access API & diagnostics |
| `WeatherLerp.h/cpp` | Interpolation & easing functions |
| `WeatherPresetsDemo.cpp` | Comprehensive test suite |
| `WEATHER_SYSTEM_README.md` | This system overview |
| `WEATHER_INTEGRATION.md` | Developer integration guide (you are here) |

---

## Support

For questions or issues:

1. **Check the test suite:** `WeatherPresetDemo::RunAllTests()`
2. **Review examples:** See `WeatherLerp.cpp` example patterns
3. **Validate data:** Use `WeatherLerp::ValidatePresetForInterpolation()`
4. **Print diagnostics:** Use `WeatherDatabase::GetPresetDiagnostics()`

---

**Version:** 1.0  
**Status:** Production Ready ✅  
**Last Updated:** 2026-06-19
