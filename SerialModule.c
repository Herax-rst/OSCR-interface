/**********************************************************************\
* Asynchronous Serial Port - by OrionSoft [2021] https://orionsoft.fr/ *
\**********************************************************************/

#include "SerialModule.h"
#include <windows.h>
#include <stdio.h>

SER_RESULT	Serial_SendBuf(sSerialModule *s, void *buf, uint32_t n)
{
	DWORD		dwWritten;
	SER_RESULT	res = SER_OK;

	if (!s->initialized)
		return (SER_ERROR);

	if (!WriteFile(s->CommHandle, buf, n, &dwWritten, &s->ovWrite))
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			printf("Serial Error: WriteFile\n");
			return (SER_ERROR);
		}
		else
		{
			while (42)
			{
				switch (WaitForSingleObject(s->ovWrite.hEvent, 0))
				{
					case WAIT_OBJECT_0:
						if (GetOverlappedResult(s->CommHandle, &s->ovWrite, &dwWritten, FALSE))
							goto done;
					break;

					case WAIT_TIMEOUT:
					break;

					default:
						printf("Serial Error: Write WaitForSingleObject\n");
						res = SER_ERROR;
						goto done;
					break;
				}
			}
		}
	}
done:
	ResetEvent(s->ovWrite.hEvent);
	return (res);
}


bool		Serial_SendStr(sSerialModule *s, char *str)
{
	printf("Serial_SendStr: %s\n", str);
	return (Serial_SendBuf(s, str, strlen(str)) == SER_OK);
}


SER_RESULT	Serial_ReadChar(sSerialModule *s)
{
	DWORD		dwRead;
	SER_RESULT	res = SER_WAIT;

	if (!s->initialized)
		return (SER_ERROR);

	if (!s->waitread)
	{
		if (!ReadFile(s->CommHandle, &s->data, 1, &dwRead, &s->ovRead))
		{
			if (GetLastError() != ERROR_IO_PENDING)
			{
				printf("Serial Error: ReadFile\n");
				return (SER_ERROR);
			}
			else
				s->waitread = true;
		}
		else
			return (SER_OK);
	}
	else
	{
		switch (WaitForSingleObject(s->ovRead.hEvent, 0))
		{
			case WAIT_OBJECT_0:
				if (GetOverlappedResult(s->CommHandle, &s->ovRead, &dwRead, FALSE))
					res = SER_OK;
				else
					res = SER_ERROR;
			break;

			case WAIT_TIMEOUT:
			break;

			default:
				// Error
				printf("Serial Error: Read WaitForSingleObject\n");
				res = SER_ERROR;
			break;
		}
	}

	if (res != SER_WAIT)
	{
		s->waitread = false;
		ResetEvent(s->ovRead.hEvent);
	}
	return (res);
}


bool	Serial_ProcessReadLine(sSerialModule *s)
{
	while (Serial_ReadChar(s) == SER_OK)	// While we have data on serial line
	{
		// Store and parse data we get from serial port
		if (s->data == '\r')		// Skip carriage return
			*s->strptr = '\0';
		else if (s->data == '\n')
		{
addline:
			printf("Serial: %s\n",s->strbuf);
			if (s->ReceiveLineCallback)
				s->ReceiveLineCallback(s->strbuf);
			s->strptr = s->strbuf;
			break;	// Don't read further serial data since we called a callback function, it might need to process some stuff before reading next serial data
		}
		else if (s->data < ' ')	// Skip any non ascii character (avoid Escape sequence)
		{
			s->strptr = s->strbuf;	// get back to the beginning
			return (false); // Special char
		}
		else
		{
			*s->strptr++ = s->data;
			if (s->strptr == &s->strptr[sizeof(s->strbuf)])
			{
				*(s->strptr - 1) = '\0';
				printf("Serial buffer overflow !\n");
				goto addline;
			}
		}
	}
	return (true);
}


SER_RESULT	Serial_SetDTR(sSerialModule *s, bool state)
{
	if (!s->initialized)
		return (SER_ERROR);
	if (!EscapeCommFunction(s->CommHandle, state ? SETDTR : CLRDTR))
		return SER_ERROR;
    return SER_OK;
}


SER_RESULT	Serial_ClearInBuf(sSerialModule *s)
{
	if (!s->initialized)
		return (SER_ERROR);
    if (!PurgeComm(s->CommHandle, PURGE_RXCLEAR))
		return SER_ERROR;
    return SER_OK;
}

void		Serial_List(void (*callback)(char *comport), int n)
{
	COMMCONFIG	cfg;
	DWORD		dwSize = sizeof(cfg);
	char		comstr[10];
	int			i;

	for (i = 1; i <= n; i++)
	{
		sprintf(comstr, "COM%d", i);
		if (GetDefaultCommConfig(comstr, &cfg, &dwSize))
			callback(comstr);
	}
}

SER_RESULT	Serial_Open(sSerialModule *s, char *comport, uint32_t baudrate, bool parity, bool flowctl)
{
	DCB				dcb;
	COMMTIMEOUTS	cto;
	char			comstr[10];

	memset(s, 0, sizeof(sSerialModule));

	if ((strlen(comport) > 5) || strncasecmp(comport, "COM", 3))
		return (SER_ERROR);

	sprintf(comstr, "\\\\.\\%s", comport);

	s->strptr = s->strbuf;
	s->SendStr = Serial_SendStr;

	s->ovRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (s->ovRead.hEvent == NULL)
		return (SER_ERROR);

	s->ovWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (s->ovWrite.hEvent == NULL)
	{
		CloseHandle(s->ovRead.hEvent);
		return (SER_ERROR);
	}

	s->CommHandle = CreateFile(comstr, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (s->CommHandle == INVALID_HANDLE_VALUE)
	{
		CloseHandle(s->ovRead.hEvent);
		CloseHandle(s->ovWrite.hEvent);
		return (SER_ERROR);
	}

	if (!GetCommState(s->CommHandle, &dcb))
		goto error;

	dcb.BaudRate		= baudrate;
	dcb.fBinary			= TRUE;
	dcb.fParity			= parity ? TRUE : FALSE;
	dcb.fOutxCtsFlow	= flowctl;
	dcb.fOutxDsrFlow	= FALSE;
	dcb.fDtrControl		= DTR_CONTROL_DISABLE;
	dcb.fRtsControl		= flowctl ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE;
	dcb.fDsrSensitivity	= FALSE;
	dcb.fOutX			= FALSE;
	dcb.fInX			= FALSE;
	dcb.fErrorChar		= FALSE;
	dcb.fNull			= FALSE;
	dcb.fAbortOnError	= FALSE;
	dcb.ByteSize		= 8;
	dcb.Parity			= parity ? EVENPARITY : NOPARITY;
	dcb.StopBits		= ONESTOPBIT;

	memset(&cto, 0, sizeof(cto));
	if (!SetCommTimeouts(s->CommHandle, &cto))
		goto error;

	if (!SetCommState(s->CommHandle, &dcb))
		goto error;

	s->initialized = true;
	return (SER_OK);

error:
	CloseHandle(s->CommHandle);
	CloseHandle(s->ovRead.hEvent);
	CloseHandle(s->ovWrite.hEvent);
	return (SER_ERROR);
}


SER_RESULT	Serial_Close(sSerialModule *s)
{
	if (!s->initialized)
		return (SER_ERROR);
	CloseHandle(s->CommHandle);
	CloseHandle(s->ovRead.hEvent);
	CloseHandle(s->ovWrite.hEvent);
	s->initialized = false;
	return (SER_OK);
}
