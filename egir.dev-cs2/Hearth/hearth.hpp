#pragma once

#include <Windows.h>
#include <cstdio>
#include <chrono>
#include <thread>

namespace Hearth {
	void __stdcall Init(HMODULE Instace);
	void __stdcall Destroy(HMODULE Instance);
	FILE* AllocConsole();
}