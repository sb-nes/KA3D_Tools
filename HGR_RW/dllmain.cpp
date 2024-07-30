//#pragma comment(lib, "engine.lib")

// Entry Point
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <crtdbg.h>

BOOL APIENTRY DllMain(HMODULE hmodule,
					  DWORD ul_reason_for_call,
					  LPVOID lpreserved) {
	switch (ul_reason_for_call){

		case DLL_PROCESS_ATTACH:
#if _DEBUG
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
		
	}
	return true;
}