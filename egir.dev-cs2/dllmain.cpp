#include <hearth.hpp>

BOOL WINAPI DllMain(
    HINSTANCE Instance,
    DWORD Reason,
    LPVOID Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE Thread = CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Hearth::Init), Instance, NULL, NULL);
		if (Thread)
		{
			CloseHandle(Thread);
		}
		break;
	}
	case DLL_PROCESS_DETACH:
		Hearth::Destroy(Instance);
		break;
	}

    return TRUE;
}