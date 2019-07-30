#pragma once

#ifdef HexControl_DLL_EXPORT
	#ifdef _WIN32
		#define DLLEXPORT __declspec(dllexport) 
	#else
		#define DLLEXPORT
	#endif
#else
	#ifdef _WIN32
		#define DLLEXPORT __declspec(dllimport) 
	#else
		#define DLLEXPORT
	#endif
#endif

namespace HexEditControl
{
	extern "C" DLLEXPORT CWnd* ShowHexControlEx(HWND hMainWnd, unsigned int x, unsigned int y, unsigned int cx, unsigned int cy, BOOL isCanMove, BOOL isCanResize);
	extern "C" DLLEXPORT CWnd* ShowHexControl(HWND hMainWnd, int x, int y, int cx, int cy);
	extern "C" DLLEXPORT void SetData(CWnd*, BYTE *data, unsigned __int64 len);
	extern "C" DLLEXPORT unsigned __int64 GetDataLength(CWnd*);
	extern "C" DLLEXPORT void GetData(CWnd*, BYTE *data, unsigned __int64 len);
}
