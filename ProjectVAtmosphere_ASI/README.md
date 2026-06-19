# Project V Atmosphere ASI

Weather State Controller for Project V Atmosphere. This builds as a ScriptHookV-compatible x64 ASI plugin and manages GTA V weather states, transitions, and exported runtime weather data.

## Build

1. Install Visual Studio 2022 with the Desktop development with C++ workload.
2. Copy the ScriptHookV SDK headers into the `sdk` folder and `ScriptHookV.lib` into `sdk\lib`.
   Required headers: `enums.h`, `main.h`, `nativeCaller.h`, `natives.h`, and `types.h`.
3. Open `ProjectVAtmosphere_ASI.sln`.
4. Select `Release | x64`.
5. Build the solution.

The compiled plugin is written to:

`bin\x64\Release\ProjectVAtmosphere.asi`

## Install

Copy `ProjectVAtmosphere.asi` into the GTA V root directory, next to `GTA5.exe` and `ScriptHookV.dll`.

## Exported Runtime State

The plugin exports:

`g_ProjectVAtmosphereWeatherState`

`g_ProjectVAtmosphereActivePreset`

External systems can read these symbols to observe the current weather IDs, transition factor, wind speed, storm intensity, and the full interpolated weather preset.
