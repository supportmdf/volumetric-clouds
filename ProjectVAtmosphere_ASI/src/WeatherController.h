#pragma once

#include "WeatherPresets.h"

struct GlobalWeatherState
{
    int currentWeather;
    int nextWeather;
    float transitionFactor;
    float windSpeed;
    float stormIntensity;
};

#if defined(PVA_BUILD)
#define PVA_API __declspec(dllexport)
#else
#define PVA_API __declspec(dllimport)
#endif

extern "C" PVA_API GlobalWeatherState g_ProjectVAtmosphereWeatherState;
extern "C" PVA_API WeatherPreset g_ProjectVAtmosphereActivePreset;
extern "C" PVA_API const WeatherPreset* PVA_GetActiveWeatherPreset();
extern "C" PVA_API const GlobalWeatherState* PVA_GetGlobalWeatherState();
extern "C" PVA_API void PVA_RequestWeather(int weatherState, float transitionSeconds);
extern "C" PVA_API void PVA_ForceWeather(int weatherState);

class WeatherController
{
public:
    void Initialize();
    void Shutdown();
    void Update();

    void RequestWeather(WeatherState state, float transitionSeconds);
    void ForceWeather(WeatherState state);

    const WeatherPreset& GetActivePreset() const;
    const GlobalWeatherState& GetGlobalState() const;

private:
    bool IsGameReady();
    void UpdateClock();
    void UpdateStateMachine();
    void UpdateTransition();
    void ApplyGameWeather();
    void PublishGlobalState();

    bool m_initialized = false;
    bool m_gameReady = false;
    bool m_transitionActive = false;

    WeatherState m_currentState = WeatherState::Clear;
    WeatherState m_nextState = WeatherState::Clear;
    WeatherState m_lastAppliedCurrentState = WeatherState::Count;
    WeatherState m_lastAppliedNextState = WeatherState::Count;

    WeatherPreset m_activePreset = {};
    WeatherPreset m_transitionStartPreset = {};

    unsigned long long m_startTickMs = 0ULL;
    unsigned long long m_lastTickMs = 0ULL;

    float m_deltaSeconds = 0.0f;
    float m_stateElapsedSeconds = 0.0f;
    float m_transitionElapsedSeconds = 0.0f;
    float m_transitionDurationSeconds = 1.0f;
    float m_transitionFactor = 0.0f;
    float m_lastAppliedTransitionFactor = -1.0f;
};

WeatherController& GetWeatherController();
