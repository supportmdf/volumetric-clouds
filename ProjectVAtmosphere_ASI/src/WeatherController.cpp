#include "WeatherController.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "main.h"
#include "nativeCaller.h"

namespace
{
    constexpr unsigned long long kNativePlayerPedId = 0xD80958FC74E988A6ULL;
    constexpr unsigned long long kNativeDoesEntityExist = 0x7239B21A38F536BAULL;
    constexpr unsigned long long kNativeSetWeatherTypeNowPersist = 0xED712CA327900C8AULL;
    constexpr unsigned long long kNativeSetWeatherTypeTransition = 0x578C752848ECFA0CULL;

    constexpr float kStartupSafetyDelaySeconds = 5.0f;
    constexpr float kDefaultStateDurationSeconds = 240.0f;
    constexpr float kDefaultTransitionDurationSeconds = 60.0f;
    constexpr float kMinimumTransitionDurationSeconds = 0.001f;
    constexpr float kNativeTransitionApplyEpsilon = 0.0025f;
    constexpr float kMaxDeltaSeconds = 0.25f;

    inline void PushNativeInt(const int value)
    {
        nativePush64(static_cast<unsigned long long>(value));
    }

    inline void PushNativeUInt(const unsigned int value)
    {
        nativePush64(static_cast<unsigned long long>(value));
    }

    inline void PushNativeFloat(const float value)
    {
        unsigned long long storage = 0ULL;
        *reinterpret_cast<float*>(&storage) = value;
        nativePush64(storage);
    }

    inline void PushNativeString(const char* value)
    {
        nativePush64(reinterpret_cast<unsigned long long>(value));
    }

    inline int NativePlayerPedId()
    {
        nativeInit(kNativePlayerPedId);
        return *reinterpret_cast<int*>(nativeCall());
    }

    inline bool NativeDoesEntityExist(const int entity)
    {
        nativeInit(kNativeDoesEntityExist);
        PushNativeInt(entity);
        return *reinterpret_cast<int*>(nativeCall()) != 0;
    }

    inline void NativeSetWeatherTypeNowPersist(const char* weatherName)
    {
        nativeInit(kNativeSetWeatherTypeNowPersist);
        PushNativeString(weatherName);
        nativeCall();
    }

    inline void NativeSetWeatherTypeTransition(const unsigned int fromWeather, const unsigned int toWeather, const float factor)
    {
        nativeInit(kNativeSetWeatherTypeTransition);
        PushNativeUInt(fromWeather);
        PushNativeUInt(toWeather);
        PushNativeFloat(PVA::Clamp01(factor));
        nativeCall();
    }

    inline float AbsFloat(const float value)
    {
        return value < 0.0f ? -value : value;
    }

    inline unsigned long long CurrentTickMs()
    {
        return static_cast<unsigned long long>(GetTickCount64());
    }
}

extern "C" PVA_API GlobalWeatherState g_ProjectVAtmosphereWeatherState =
{
    static_cast<int>(WeatherState::Clear),
    static_cast<int>(WeatherState::Clear),
    0.0f,
    0.0f,
    0.0f
};

extern "C" PVA_API WeatherPreset g_ProjectVAtmosphereActivePreset = {};

extern "C" PVA_API const WeatherPreset* PVA_GetActiveWeatherPreset()
{
    return &g_ProjectVAtmosphereActivePreset;
}

extern "C" PVA_API const GlobalWeatherState* PVA_GetGlobalWeatherState()
{
    return &g_ProjectVAtmosphereWeatherState;
}

extern "C" PVA_API void PVA_RequestWeather(const int weatherState, const float transitionSeconds)
{
    if (IsValidWeatherState(weatherState))
    {
        GetWeatherController().RequestWeather(static_cast<WeatherState>(weatherState), transitionSeconds);
    }
}

extern "C" PVA_API void PVA_ForceWeather(const int weatherState)
{
    if (IsValidWeatherState(weatherState))
    {
        GetWeatherController().ForceWeather(static_cast<WeatherState>(weatherState));
    }
}

WeatherController& GetWeatherController()
{
    static WeatherController controller;
    return controller;
}

void WeatherController::Initialize()
{
    m_startTickMs = CurrentTickMs();
    m_lastTickMs = m_startTickMs;
    m_deltaSeconds = 0.0f;
    m_stateElapsedSeconds = 0.0f;
    m_transitionElapsedSeconds = 0.0f;
    m_transitionDurationSeconds = kDefaultTransitionDurationSeconds;
    m_transitionFactor = 0.0f;
    m_currentState = WeatherState::Clear;
    m_nextState = WeatherState::Clear;
    m_lastAppliedCurrentState = WeatherState::Count;
    m_lastAppliedNextState = WeatherState::Count;
    m_lastAppliedTransitionFactor = -1.0f;
    m_activePreset = GetWeatherPreset(m_currentState);
    m_transitionStartPreset = m_activePreset;
    m_initialized = true;
    m_gameReady = false;
    m_transitionActive = false;

    PublishGlobalState();
}

void WeatherController::Shutdown()
{
    m_initialized = false;
    m_gameReady = false;
    m_transitionActive = false;
    PublishGlobalState();
}

void WeatherController::Update()
{
    if (!m_initialized)
    {
        Initialize();
    }

    UpdateClock();

    if (!m_gameReady)
    {
        m_gameReady = IsGameReady();
        PublishGlobalState();
        return;
    }

    UpdateStateMachine();
    UpdateTransition();
    ApplyGameWeather();
    PublishGlobalState();
}

void WeatherController::RequestWeather(const WeatherState state, const float transitionSeconds)
{
    if (state == m_nextState && m_transitionActive)
    {
        return;
    }

    if (state == m_currentState && !m_transitionActive)
    {
        return;
    }

    m_nextState = state;
    m_transitionStartPreset = m_activePreset;
    m_transitionActive = true;
    m_transitionElapsedSeconds = 0.0f;
    m_transitionFactor = 0.0f;
    m_transitionDurationSeconds = transitionSeconds > kMinimumTransitionDurationSeconds ? transitionSeconds : kMinimumTransitionDurationSeconds;
    m_stateElapsedSeconds = 0.0f;
}

void WeatherController::ForceWeather(const WeatherState state)
{
    m_currentState = state;
    m_nextState = state;
    m_transitionActive = false;
    m_transitionElapsedSeconds = 0.0f;
    m_transitionDurationSeconds = kDefaultTransitionDurationSeconds;
    m_transitionFactor = 0.0f;
    m_stateElapsedSeconds = 0.0f;
    m_activePreset = GetWeatherPreset(m_currentState);
    m_transitionStartPreset = m_activePreset;
    m_lastAppliedCurrentState = WeatherState::Count;
    m_lastAppliedNextState = WeatherState::Count;
    m_lastAppliedTransitionFactor = -1.0f;

    PublishGlobalState();
}

const WeatherPreset& WeatherController::GetActivePreset() const
{
    return m_activePreset;
}

const GlobalWeatherState& WeatherController::GetGlobalState() const
{
    return g_ProjectVAtmosphereWeatherState;
}

bool WeatherController::IsGameReady()
{
    const unsigned long long elapsedMs = CurrentTickMs() - m_startTickMs;
    if (elapsedMs < static_cast<unsigned long long>(kStartupSafetyDelaySeconds * 1000.0f))
    {
        return false;
    }

    __try
    {
        const int playerPed = NativePlayerPedId();
        return playerPed != 0 && NativeDoesEntityExist(playerPed);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

void WeatherController::UpdateClock()
{
    const unsigned long long currentTick = CurrentTickMs();
    const unsigned long long elapsedMs = currentTick >= m_lastTickMs ? (currentTick - m_lastTickMs) : 0ULL;
    m_lastTickMs = currentTick;

    m_deltaSeconds = static_cast<float>(elapsedMs) * 0.001f;
    if (m_deltaSeconds > kMaxDeltaSeconds)
    {
        m_deltaSeconds = kMaxDeltaSeconds;
    }
}

void WeatherController::UpdateStateMachine()
{
    if (m_transitionActive)
    {
        return;
    }

    m_stateElapsedSeconds += m_deltaSeconds;
    if (m_stateElapsedSeconds >= kDefaultStateDurationSeconds)
    {
        RequestWeather(GetNextAutomaticWeatherState(m_currentState), kDefaultTransitionDurationSeconds);
    }
}

void WeatherController::UpdateTransition()
{
    if (!m_transitionActive)
    {
        m_transitionFactor = 0.0f;
        m_activePreset = GetWeatherPreset(m_currentState);
        return;
    }

    m_transitionElapsedSeconds += m_deltaSeconds;
    m_transitionFactor = PVA::Clamp01(m_transitionElapsedSeconds / m_transitionDurationSeconds);

    InterpolateWeatherPresets(m_transitionStartPreset, GetWeatherPreset(m_nextState), m_transitionFactor, m_activePreset);

    if (m_transitionFactor >= 1.0f)
    {
        m_currentState = m_nextState;
        m_transitionActive = false;
        m_transitionElapsedSeconds = 0.0f;
        m_transitionFactor = 0.0f;
        m_stateElapsedSeconds = 0.0f;
        m_activePreset = GetWeatherPreset(m_currentState);
        m_transitionStartPreset = m_activePreset;
    }
}

void WeatherController::ApplyGameWeather()
{
    __try
    {
        if (m_transitionActive)
        {
            const bool stateChanged = m_currentState != m_lastAppliedCurrentState || m_nextState != m_lastAppliedNextState;
            const bool factorChanged = AbsFloat(m_transitionFactor - m_lastAppliedTransitionFactor) >= kNativeTransitionApplyEpsilon;

            if (stateChanged || factorChanged)
            {
                NativeSetWeatherTypeTransition(GetNativeWeatherHash(m_currentState), GetNativeWeatherHash(m_nextState), m_transitionFactor);
                m_lastAppliedCurrentState = m_currentState;
                m_lastAppliedNextState = m_nextState;
                m_lastAppliedTransitionFactor = m_transitionFactor;
            }

            return;
        }

        if (m_currentState != m_lastAppliedCurrentState || m_lastAppliedNextState != m_nextState)
        {
            NativeSetWeatherTypeNowPersist(GetNativeWeatherName(m_currentState));
            m_lastAppliedCurrentState = m_currentState;
            m_lastAppliedNextState = m_nextState;
            m_lastAppliedTransitionFactor = -1.0f;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        m_gameReady = false;
    }
}

void WeatherController::PublishGlobalState()
{
    g_ProjectVAtmosphereActivePreset = m_activePreset;
    g_ProjectVAtmosphereWeatherState.currentWeather = static_cast<int>(m_currentState);
    g_ProjectVAtmosphereWeatherState.nextWeather = static_cast<int>(m_nextState);
    g_ProjectVAtmosphereWeatherState.transitionFactor = m_transitionFactor;
    g_ProjectVAtmosphereWeatherState.windSpeed = m_activePreset.windSpeed;
    g_ProjectVAtmosphereWeatherState.stormIntensity = m_activePreset.stormIntensity;
}
