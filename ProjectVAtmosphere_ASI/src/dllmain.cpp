#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "main.h"
#include "WeatherController.h"

namespace
{
    void ScriptMain()
    {
        WeatherController& controller = GetWeatherController();
        controller.Initialize();

        while (true)
        {
            controller.Update();
            WAIT(0);
        }
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    (void)reserved;

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(module);
        scriptRegister(module, ScriptMain);
        break;

    case DLL_PROCESS_DETACH:
        GetWeatherController().Shutdown();
        scriptUnregister(module);
        break;

    default:
        break;
    }

    return TRUE;
}
