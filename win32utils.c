/***************************************************************\
* Win32 API Utils - OrionSoft [2018-2020] https://orionsoft.fr/ *
\***************************************************************/

#include "win32utils.h"
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>

/****************************************/

void	*SystemGetFile(char *filename, int *sizeptr)
{
	FILE	*f;
	int		size;
	void	*buffer = NULL;

	if ((f = fopen(filename, "rb")))
	{
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		if (sizeptr)
			*sizeptr = size;
		if ((buffer = malloc(size)))
			fread(buffer, 1, size, f);
		fclose(f);
	}

	return (buffer);
}

/****************************************/

BOOL	DoDialogOpenFile(HWND hwnd, char *type, char *filename, int maxsize)
{
	OPENFILENAME	ofn;

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = maxsize;
	ofn.lpstrFilter = type;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileName(&ofn))
		return (TRUE);

	return (FALSE);
}

/****************************************/

BOOL	DoDialogSaveFile(HWND hwnd, char *type, char *ext, char *filename, int maxsize)
{
	OPENFILENAME	ofn;

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filename;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = maxsize;
	ofn.lpstrFilter = type;
	ofn.lpstrDefExt = ext;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetSaveFileName(&ofn))
		return (TRUE);

	return (FALSE);
}

/****************************************/

char	*ExtractFilenameFromFullPath(char *fullpath)
{
	char	*ptr = fullpath;

	while (*ptr)	// Go to the end of String
		ptr++;
	while ((ptr != fullpath) && (*ptr != '\\'))	// Rollback to last '\' or (start or string)
		ptr--;
	if (*ptr == '\\')	// Skip '\'
		ptr++;
	return (ptr);
}

char	*ExtractExtFromFilename(char *filename)
{
	char	*ptr = filename;

	while (*ptr)	// Go to the end of String
		ptr++;
	while ((ptr != filename) && (*ptr != '.'))	// Rollback to last '.' or (start or string)
		ptr--;
	if (*ptr == '.')	// Skip '.'
		ptr++;
	return (ptr);
}

char	*ExtractFilenameFromFullPathWithoutExt(char *filename)
{
	char	*ptr = ExtractFilenameFromFullPath(filename);

	*(ExtractExtFromFilename(ptr) - 1) = '\0';
	return (ptr);
}

/****************************************/

void	ComboBox_SetCurSelFromString(HWND dialoghwnd, int idc, char *str)
{
	HWND hwnd = GetDlgItem(dialoghwnd, idc);

	ComboBox_SetCurSel(hwnd, ComboBox_FindStringExact(hwnd, 0, str));
}

void	ComboBox_GetCurSelStr(HWND combobox, char *buffer)
{
	int	index = ComboBox_GetCurSel(combobox);

	if (index >= 0)
		ComboBox_GetLBText(combobox, index, buffer);
}

void	ListBox_SetCurSelFromString(HWND dialoghwnd, int idc, char *str)
{
	HWND hwnd = GetDlgItem(dialoghwnd, idc);

	ListBox_SetCurSel(hwnd, ListBox_FindStringExact(hwnd, 0, str));
}

void	ListBox_GetCurSelStr(HWND combobox, char *buffer)
{
	int	index = ListBox_GetCurSel(combobox);

	if (index >= 0)
		ListBox_GetText(combobox, index, buffer);
}
