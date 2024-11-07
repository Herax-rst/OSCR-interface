/******************************************************\
* OSCR GUI - by OrionSoft [2024] https://orionsoft.fr/ *
\******************************************************/

#ifndef	_MAIN_H_
#define	_MAIN_H_

#define WIN32_LEAN_AND_MEAN
#undef	WINVER
#define WINVER 0x0500

#include <windows.h>
#include <wingdi.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "resource.h"
#include "win32utils.h"

#include "SerialModule.h"

extern	sSerialModule	SerialModule;
extern	HWND			DialogHwnd;

#endif
