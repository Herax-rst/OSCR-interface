/**************************************************************\
* Win32 OBJ - by OrionSoft [2020] https://orionsoft.fr/        *
* Win32 window control processing using object based callback. *
\**************************************************************/

#ifndef	_WIN32OBJ_H_
#define	_WIN32OBJ_H_

#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdint.h>

// Usage:
// Define a list of object of the type "sW32OBJ" with at least "idc" and "callback" members.
// Call the function "W32OBJ_InitList" with the Win32 window handle, a pointer to this object list and the number of objects.
// Call the function "W32OBJ_ProcessList" in your "DialogProc" function in the WM_COMMAND / WM_HSCROLL / WM_VSCROLL message handlers.

struct sdW32OBJ
{
	int			idc;												// IDC define from resource.h
    void		(*callback)(struct sdW32OBJ *obj, uint32_t data);	// Function called when a Button is clicked, an Edit is modified or Slider/ComboBox/ListBox selection change (data is the new index)
	void		*user_data;											// A pointer available to you so you can use it in the callback function.

    // Members below are initialized by W32OBJ_InitList
    HWND	hwnd;	// Win32 handle of the window control
    int		type;	// Internal type of the control
	void	(*SetText)(struct sdW32OBJ *obj, char *text);							// Specific function to set the text of this control
	void	(*GetText)(struct sdW32OBJ *obj, char *text_buffer, int buffer_size);	// Can be used to get the text from an Edit or the current selected item in a ComboBox/ListBox
};

typedef struct sdW32OBJ	sW32OBJ;

void	W32OBJ_InitList(HWND dlg_hwnd, sW32OBJ *list, int n_objects);	// Call this first with your sW32OBJ list.
BOOL	W32OBJ_ProcessList(UINT uMsg, WPARAM wParam, LPARAM lParam);	// Call this from your "DialogProc" function in the WM_COMMAND / WM_HSCROLL / WM_VSCROLL message handlers.
sW32OBJ	*W32OBJ_GetObjFromIDC(HWND dlg_hwnd, int idc);					// Return a sW32OBJ pointer from the list using the IDC define

#endif
