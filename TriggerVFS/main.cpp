#include "TriggerVFS.h"
#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
	switch(fdwReason)
	{
	case DLL_PROCESS_DETACH:
		{
#ifdef _DEBUG
			_CrtDumpMemoryLeaks();
#endif
		}break;
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE; //DLL successfully attached!
}