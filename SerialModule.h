/**********************************************************************\
* Asynchronous Serial Port - by OrionSoft [2021] https://orionsoft.fr/ *
\**********************************************************************/

#ifndef	_SERIAL_MODULE_H_
#define	_SERIAL_MODULE_H_

#include <stdint.h>
#include <stdbool.h>
#include <windows.h>

typedef int32_t	SER_RESULT;

#define	SER_OK		0
#define	SER_WAIT	1
#define	SER_ERROR	(-1)

typedef struct sSerialModule
{
	HANDLE		CommHandle;
	OVERLAPPED	ovRead;
	OVERLAPPED	ovWrite;
	bool		initialized;
	bool		waitread;
	bool		waitwrite;
	char		data;
	bool		(*SendStr)(struct sSerialModule *s, char *str);
	void		(*ReceiveLineCallback)(char *str);
	char		strbuf[256];
	char		*strptr;
}	sSerialModule;

void		Serial_List(void (*callback)(char *comport), int n);
SER_RESULT	Serial_SendBuf(sSerialModule *s, void *buf, uint32_t n);
bool		Serial_SendStr(sSerialModule *s, char *str);
SER_RESULT	Serial_ReadChar(sSerialModule *s);
bool		Serial_ProcessReadLine(sSerialModule *s);	// Non blocking, will call ReceiveLineCallback as soon as it receive a line "\r\n" terminated
SER_RESULT	Serial_SetDTR(sSerialModule *s, bool state);
SER_RESULT	Serial_ClearInBuf(sSerialModule *s);
SER_RESULT	Serial_Open(sSerialModule *s, char *comport, uint32_t baudrate, bool parity, bool flowctl);
SER_RESULT	Serial_Close(sSerialModule *s);

#endif
