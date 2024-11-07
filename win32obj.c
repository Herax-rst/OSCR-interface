/**************************************************************\
* Win32 OBJ - by OrionSoft [2020] https://orionsoft.fr/        *
* Win32 window control processing using object based callback. *
\**************************************************************/

#include "win32obj.h"
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>

static sW32OBJ *W32OBJ_list;
static int		W32OBJ_n_objects;

enum
{
	W32OBJ_TYPE_NONE,
	W32OBJ_TYPE_BUTTON,
	W32OBJ_TYPE_EDIT,
	W32OBJ_TYPE_TEXT,
	W32OBJ_TYPE_LISTBOX,
	W32OBJ_TYPE_COMBOBOX,
	W32OBJ_TYPE_SLIDER
};

/****************************************/

void	iW32OBJ_SetText(sW32OBJ *obj, char *text)
{
	SendMessage(obj->hwnd, WM_SETTEXT, 0, (LPARAM)text);
}

void	iW32OBJ_GetText(sW32OBJ *obj, char *text_buffer, int buffer_size)
{
	SendMessage(obj->hwnd, WM_GETTEXT, (WPARAM)buffer_size, (LPARAM)text_buffer);
}

void	iW32OBJ_ListBoxGetText(sW32OBJ *obj, char *text_buffer, int buffer_size)
{
	int	index = ListBox_GetCurSel(obj->hwnd);

	if (index >= 0)
		ListBox_GetText(obj->hwnd, index, text_buffer);
}

void	iW32OBJ_ComboBoxGetText(sW32OBJ *obj, char *text_buffer, int buffer_size)
{
	int	index = ComboBox_GetCurSel(obj->hwnd);

	if (index >= 0)
		ComboBox_GetLBText(obj->hwnd, index, text_buffer);
}

/****************************************/

void	W32OBJ_InitList(HWND dlg_hwnd, sW32OBJ *list, int n_objects)
{
	int		i;
	char	str[128];

	W32OBJ_list = list;
	W32OBJ_n_objects = n_objects;

	for (i = 0; i < n_objects; i++, list++)
	{
		list->hwnd = GetDlgItem(dlg_hwnd, list->idc);
		GetClassName(list->hwnd, str, sizeof(str));
		if (!strcmp(str, "Button"))	// Can also be a GroupBox
		{
			list->type = W32OBJ_TYPE_BUTTON;
			list->SetText = iW32OBJ_SetText;
		}
		else if (!strcmp(str, "Edit"))
		{
			list->type = W32OBJ_TYPE_EDIT;
			list->SetText = iW32OBJ_SetText;
			list->GetText = iW32OBJ_GetText;
		}
		else if (!strcmp(str, "Static"))
		{
			list->type = W32OBJ_TYPE_TEXT;
			list->SetText = iW32OBJ_SetText;
		}
		else if (!strcmp(str, "ListBox"))
		{
			list->type = W32OBJ_TYPE_LISTBOX;
			list->GetText = iW32OBJ_ListBoxGetText;
		}
		else if (!strcmp(str, "ComboBox"))
		{
			list->type = W32OBJ_TYPE_COMBOBOX;
			list->GetText = iW32OBJ_ComboBoxGetText;
		}
		else if (strstr(str, "trackbar"))
		{
			list->type = W32OBJ_TYPE_SLIDER;
		}
	}
}

BOOL	W32OBJ_ProcessList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int		i;
	BOOL	process = FALSE;
	sW32OBJ *list = W32OBJ_list;

	switch (uMsg)
	{
		case WM_HSCROLL:
			if (LOWORD(wParam) == TB_ENDTRACK)	// W32OBJ_TYPE_SLIDER
				process = TRUE;
		break;

		case WM_COMMAND:
			process = TRUE;
		break;
	}

	if (list && process)
	{
		for (i = 0; i < W32OBJ_n_objects; i++, list++)
		{
			if (list->hwnd == (HWND)lParam)
			{
				if (list->callback)
				{
					switch (list->type)
					{
						case W32OBJ_TYPE_BUTTON:
							if (HIWORD(wParam) == BN_CLICKED)
								list->callback(list, 0);
						break;

						case W32OBJ_TYPE_EDIT:
							if (HIWORD(wParam) == EN_CHANGE)
								list->callback(list, 0);
						break;

						case W32OBJ_TYPE_LISTBOX:
							if (HIWORD(wParam) == LBN_SELCHANGE)
								list->callback(list, ListBox_GetCurSel(list->hwnd));
						break;

						case W32OBJ_TYPE_COMBOBOX:
							if (HIWORD(wParam) == CBN_SELENDOK)
								list->callback(list, ComboBox_GetCurSel(list->hwnd));
						break;

						case W32OBJ_TYPE_SLIDER:
							list->callback(list, SendMessage(list->hwnd, TBM_GETPOS, 0, 0));
						break;
					}
				}
				return (TRUE);
			}
		}
	}
	return (FALSE);
}

sW32OBJ	*W32OBJ_GetObjFromIDC(HWND dlg_hwnd, int idc)
{
	sW32OBJ *list = W32OBJ_list;

	if (list)
	{
		int		i;
		HWND	obj_hwnd = GetDlgItem(dlg_hwnd, idc);

		for (i = 0; i < W32OBJ_n_objects; i++, list++)
			if (list->hwnd == obj_hwnd)
				return (list);
	}

	return (NULL);
}
