#include <hearth.hpp>
#include <var.hpp>
#include <hooks.hpp>

void __stdcall Hearth::Init(HMODULE Instance)
{
	FILE* ConsoleBuffer = nullptr;
	if (var::debugConsole) ConsoleBuffer = Hearth::AllocConsole();

	DisableThreadLibraryCalls(Instance);
	if (ChecktDirectXVersion(DirectXVersion.D3D11) == true) {
		Process::Module = Instance;
		CreateThread(0, 0, Hooks::MainThread, 0, 0, 0);
	}

	while (!GetAsyncKeyState(VK_END) & 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if (var::debugConsole)
		if (ConsoleBuffer)
			fclose(ConsoleBuffer);
	FreeLibraryAndExitThread(Instance, 0);
}

FILE* Hearth::AllocConsole()
{
	::AllocConsole() && ::AttachConsole(GetCurrentProcessId());
	FILE* ConsoleBuffer = nullptr;
	freopen_s(&ConsoleBuffer, "CONOUT$", "w", stdout);
	SetConsoleTitle("egirl.dev Console DEBUG");
	return ConsoleBuffer;
}

void __stdcall Hearth::Destroy(HMODULE Instance)
{
	Hooks::Destroy();
	FreeLibraryAndExitThread(Instance, TRUE);
	if (var::debugConsole) FreeConsole();
}