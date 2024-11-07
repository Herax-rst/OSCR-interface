/******************************************************\
* OSCR GUI - by OrionSoft [2024] https://orionsoft.fr/ *
\******************************************************/

#include "main.h"
#include "win32obj.h"
#include "utils.h"
#include "SerialModule.h"
#include "transfer_protocol.h"

HINSTANCE	hInst;
HWND		DialogHwnd;

sSerialModule	SerialModule;

#define MAX_CHOICES 10

int     CHOICE_BTN_ID[MAX_CHOICES] = {IDC_BTN_CHOICE_0,IDC_BTN_CHOICE_1,IDC_BTN_CHOICE_2,IDC_BTN_CHOICE_3,IDC_BTN_CHOICE_4,IDC_BTN_CHOICE_5,IDC_BTN_CHOICE_6,IDC_BTN_CHOICE_7,IDC_BTN_CHOICE_8,IDC_BTN_CHOICE_9};
char    ChoiceButtonChar[] = "0123456789";
int     PreviousChoice = -1;

//****************************************
// Config INI

char	*Ini_SearchToken(char *ini_token, char *linestr)
{
	char	str[32];

	strcpy(str, ini_token);
	strcat(str, "=");
	return (StrCmpRef(str, linestr));
}

void	Ini_SetComboBoxSelFromConfigToken(char *ini_token, char *linestr, int combobox_idc)
{
	char	*ptr = Ini_SearchToken(ini_token, linestr);

	if (ptr)
		ComboBox_SetCurSelFromString(DialogHwnd, combobox_idc, ptr);
}

void	Ini_SaveConfigTokenFromComboBoxSel(char *ini_token, FILE *f, int combobox_idc)
{
	char	str[32];

	ComboBox_GetCurSelStr(GetDlgItem(DialogHwnd, combobox_idc), str);
	fprintf(f, "%s=%s\n", ini_token, str);
}

void	LoadConfig(void)
{
	char	str[128], *ptr;
	FILE	*f = fopen("config.ini", "r");

	if (!f)
		return;
	while (fgets(str, sizeof(str), f))
	{
		ptr = strchr(str, '\n');
		if (ptr)
			*ptr = '\0';
		Ini_SetComboBoxSelFromConfigToken("SerialOSCR", str, IDC_LIST_SERIAL);
		Ini_SetComboBoxSelFromConfigToken("SerialBauds", str, IDC_LIST_SERIAL_BAUDS);
	}
	fclose(f);
}

void	SaveConfig(void)
{
	FILE	*f = fopen("config.ini", "w");

	if (!f)
		return;
	Ini_SaveConfigTokenFromComboBoxSel("SerialOSCR", f, IDC_LIST_SERIAL);
	Ini_SaveConfigTokenFromComboBoxSel("SerialBauds", f, IDC_LIST_SERIAL_BAUDS);
	fclose(f);
}

/****************************************/
// Dialog Utils / Processing

void	OSCR_InfoAddLine(char *line)
{
	ListBox_AddString(GetDlgItem(DialogHwnd, IDC_LIST_INFO), line);
}

void	Serial_List_Callback(char *comstr)
{
	ComboBox_AddString(GetDlgItem(DialogHwnd, IDC_LIST_SERIAL), comstr);
}

void    OSCR_ReceiveLine(char *str)
{
    // Check if text line received from OSCR is an info line or a menu choice
    if ((str[0] >= '0') && (str[0] <= '9') && (str[1] >= ')') && ((str[0]-'0') == (PreviousChoice+1)))
    {
        // If menu choice, then add the choice to the buttons list
        HWND    btn = GetDlgItem(DialogHwnd, CHOICE_BTN_ID[str[0]-'0']);

        PreviousChoice++;
        Button_SetText(btn, &str[2]);
        Button_Enable(btn, TRUE);
    }
    else if (!strncmp(str, "Press", 5))
    {
        HWND    btn = GetDlgItem(DialogHwnd, IDC_BTN_CHOICE_0);

        Button_SetText(btn, str);
        Button_Enable(btn, TRUE);
    }
    else
        OSCR_InfoAddLine(str);
}

/****************************************/
// Window Objects Callbacks

void	SerialPortDetect(sW32OBJ *obj, uint32_t data)
{
	ComboBox_ResetContent(GetDlgItem(DialogHwnd, IDC_LIST_SERIAL));
	Serial_List(Serial_List_Callback, 20);
	LoadConfig();
}

void	SerialConnect(sW32OBJ *obj, uint32_t data)
{
	char	str[256];
	BOOL    ret;
	int     value;

	ComboBox_GetCurSelStr(GetDlgItem(DialogHwnd, IDC_LIST_SERIAL), str);
	value = GetItemInt(DialogHwnd, IDC_LIST_SERIAL_BAUDS, &ret);
	if (!ret)
	{
		MessageBox(DialogHwnd, "Serial bauds speed incorrect!", "Error", MB_ICONERROR);
		return;
	}

	if (Serial_Open(&SerialModule, str, value, 0, 0) != SER_OK)
	{
		MessageBox(DialogHwnd, "Cannot open selected serial port!", "Error", MB_ICONERROR);
		return;
	}

	EnableWindow(obj->hwnd, FALSE);	// Disable itself
	SaveConfig();
	SerialModule.ReceiveLineCallback = OSCR_ReceiveLine;
}

void	ChoiceButton(sW32OBJ *obj, uint32_t data)
{
    int i;

    // Disable all choice buttons
    for (i = 0; i < MAX_CHOICES; i++)
    {
        HWND    btn = GetDlgItem(DialogHwnd, CHOICE_BTN_ID[i]);

        Button_SetText(btn, "");
        Button_Enable(btn, FALSE);
    }
    PreviousChoice = -1;
    ListBox_ResetContent(GetDlgItem(DialogHwnd, IDC_LIST_INFO));
    Serial_SendBuf(&SerialModule, obj->user_data, 1);   // Send choice to serial
}

/****************************************/

sW32OBJ	WindowObjectList[] =
{
	{IDC_BTN_REDETECT, SerialPortDetect},
	{IDC_BTN_CONNECT, SerialConnect},
	{IDC_BTN_CHOICE_0, ChoiceButton, &ChoiceButtonChar[0]},
	{IDC_BTN_CHOICE_1, ChoiceButton, &ChoiceButtonChar[1]},
	{IDC_BTN_CHOICE_2, ChoiceButton, &ChoiceButtonChar[2]},
	{IDC_BTN_CHOICE_3, ChoiceButton, &ChoiceButtonChar[3]},
	{IDC_BTN_CHOICE_4, ChoiceButton, &ChoiceButtonChar[4]},
	{IDC_BTN_CHOICE_5, ChoiceButton, &ChoiceButtonChar[5]},
	{IDC_BTN_CHOICE_6, ChoiceButton, &ChoiceButtonChar[6]},
	{IDC_BTN_CHOICE_7, ChoiceButton, &ChoiceButtonChar[7]},
	{IDC_BTN_CHOICE_8, ChoiceButton, &ChoiceButtonChar[8]},
	{IDC_BTN_CHOICE_9, ChoiceButton, &ChoiceButtonChar[9]}
};

/****************************************/
// Main Dialog Processing

BOOL CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
		    HWND    baudslist;

			DialogHwnd = hwnd;

			W32OBJ_InitList(hwnd, WindowObjectList, sizeof(WindowObjectList) / sizeof(sW32OBJ));

			baudslist = GetDlgItem(DialogHwnd, IDC_LIST_SERIAL_BAUDS);
			ComboBox_AddString(baudslist, "9600");
			ComboBox_AddString(baudslist, "115200");
			ComboBox_AddString(baudslist, "250000");
			ComboBox_AddString(baudslist, "500000");
			ComboBox_AddString(baudslist, "1000000");
			SetProgressBarRange(DialogHwnd, IDC_PROGRESS, 0, 100);

			SerialPortDetect(NULL, 0);
			SetTimer(DialogHwnd, 0, 20, (TIMERPROC)NULL);   // 20ms

			return TRUE;
		}

		case WM_CLOSE:
			EndDialog(hwnd, 0);
		return TRUE;

		case WM_DESTROY:
			KillTimer(hwnd, 0);
			Serial_Close(&SerialModule);
		return TRUE;

		case WM_TIMER:		// wParam = Timer ID
			if (!Serial_ProcessReadLine(&SerialModule))
			{
                if (SerialModule.data == 16)    // Escape char from data transfer protocol
				{
					KillTimer(hwnd, 0);
                    DataTransferProtocol();
                    SetTimer(hwnd, 0, 20, (TIMERPROC)NULL);   // 20ms
				}
			}
		return FALSE;

		//*****************
		// Dialog Controls
		case WM_HSCROLL:
			W32OBJ_ProcessList(uMsg, wParam, lParam);
		break;

		case WM_COMMAND:
			if (!W32OBJ_ProcessList(uMsg, wParam, lParam))
			switch (LOWORD(wParam))
			{
				case IDC_BTN_EXIT:
					EndDialog(hwnd, 0);
				break;
			}
		return TRUE;
	}
	return FALSE;
}

/**/

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst = hInstance;
    return DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DialogProc);
}
