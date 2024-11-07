/***************************************************************\
* Win32 API Utils - OrionSoft [2018-2020] https://orionsoft.fr/ *
\***************************************************************/

#ifndef	_WIN32UTILS_H_
#define	_WIN32UTILS_H_

#define	WIN32_LEAN_AND_MEAN		// Reduce Exe size a bit
#undef	_WIN32_WINNT
#define _WIN32_WINNT	0x0502	// Windows XP SP2 minimum (KEY_WOW64_64KEY registry)

#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <commctrl.h>
#include <commdlg.h>

#define	GetSliderPosition(_HWND_dialog_, _resid_)					SendMessage(GetDlgItem(_HWND_dialog_, _resid_), TBM_GETPOS, 0, 0)	// Use in WM_HSCROLL
#define	SetSliderPosition(_HWND_dialog_, _resid_, _pos_)			SendMessage(GetDlgItem(_HWND_dialog_, _resid_), TBM_SETPOS, TRUE, (LPARAM)_pos_)
#define	SetSliderRange(_HWND_dialog_, _resid_, _start_, _end_)		SendMessage(GetDlgItem(_HWND_dialog_, _resid_), TBM_SETRANGE, TRUE, MAKELONG((_start_), (_end_)))

#ifndef PBM_SETSTATE
#define PBM_SETSTATE            (WM_USER+16) // wParam = PBST_[State] (NORMAL, ERROR, PAUSED)
#define PBST_NORMAL             0x0001
#define PBST_ERROR              0x0002
#define PBST_PAUSED             0x0003
#endif

#define	SetProgressBarRange(_HWND_dialog_, _resid_, _start_, _end_)	SendMessage(GetDlgItem(_HWND_dialog_, _resid_), PBM_SETRANGE, 0, MAKELONG((_start_), (_end_)))
#define	SetProgressBarPosition(_HWND_dialog_, _resid_, _pos_)		SendMessage(GetDlgItem(_HWND_dialog_, _resid_), PBM_SETPOS, _pos_, 0)
#define	SetProgressBarState(_HWND_dialog_, _resid_, _state_)		SendMessage(GetDlgItem(_HWND_dialog_, _resid_), PBM_SETSTATE, _state_, 0)

#define	LimitEditText(_HWND_dialog_, _resid_, _limit_)				SendMessage(GetDlgItem(_HWND_dialog_, _resid_), EM_LIMITTEXT, (WPARAM)_limit_, 0)

#define	SetItemText(_HWND_dialog_, _resid_, _text_)		SetDlgItemTextA(_HWND_dialog_, _resid_, _text_)
#define	SetItemInt(_HWND_dialog_, _resid_, _value_)		SetDlgItemInt(_HWND_dialog_, _resid_, _value_, FALSE)

#define	GetItemText(_HWND_dialog_, _resid_, _text_ptr_)	GetDlgItemTextA(_HWND_dialog_, _resid_, _text_ptr_, sizeof(_text_ptr_))	// If == 0 -> Error
#define	GetItemInt(_HWND_dialog_, _resid_, _BOOL_ptr_)	GetDlgItemInt(_HWND_dialog_, _resid_, _BOOL_ptr_, FALSE)			// If _BOOL_ptr_ == FALSE -> Error

void	*SystemGetFile(char *filename, int *sizeptr);

// DoDialogOpenFile(DialogHwnd, "Text File\0*.txt\0\0", buffer, sizeof(buffer));
BOOL	DoDialogOpenFile(HWND hwnd, char *type, char *filename, int maxsize);

// DoDialogSaveFile(DialogHwnd, "Text File\0*.txt\0\0", "txt", buffer, sizeof(buffer));
BOOL	DoDialogSaveFile(HWND hwnd, char *type, char *ext, char *filename, int maxsize);

char	*ExtractFilenameFromFullPath(char *fullpath);
char	*ExtractExtFromFilename(char *filename);
char	*ExtractFilenameFromFullPathWithoutExt(char *fullpath);

void	ComboBox_SetCurSelFromString(HWND dialoghwnd, int idc, char *str);
void	ComboBox_GetCurSelStr(HWND combobox, char *buffer);
void	ListBox_SetCurSelFromString(HWND dialoghwnd, int idc, char *str);
void	ListBox_GetCurSelStr(HWND combobox, char *buffer);

#endif
