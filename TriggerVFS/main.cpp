#include "TriggerVFS.h"
#ifdef USE_VLD // add this to preprocessor definitions if you have visual leak detector installed
#include <vld.h>
#endif

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
	switch(fdwReason)
	{
	case DLL_PROCESS_DETACH:
		{
#ifdef USE_VLD
			VLDReportLeaks();
#endif
		}break;
	case DLL_PROCESS_ATTACH:
		{
#ifdef USE_VLD
			VLDEnable();
#endif
		}break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE; //DLL successfully attached!
}