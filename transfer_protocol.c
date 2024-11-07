/***************************************************************************\
* OSCR Serial Transfer Protocol - by OrionSoft [2024] https://orionsoft.fr/ *
\***************************************************************************/

#include "transfer_protocol.h"

FILE	*PCdrvFD = NULL;
char	*lastFileName = NULL;

/****************************************/

static const uint8_t CRC8_Tab[256] =
{
	  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
	157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
	 35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
	190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
	 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
	219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
	101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
	248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
	140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
	 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
	175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
	 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
	202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
	 87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
	233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
	116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};

uint8_t		CRC8_Compute(uint8_t *data, uint32_t size)
{
	uint8_t	crc = 0xFF;

	while (size--)
		crc = CRC8_Tab[crc ^ (*data++)];

	return (crc);
}

/****************************************/

enum
{
	CMD_UNKNOWN,
	CMD_UPLOAD,
	CMD_DOWNLOAD
};

uint32_t	atohex(char *str)
{
	uint32_t	value = 0;

	if ((str[0] == '0') && (str[1] == 'x'))
		str = &str[2];
	while (42)
	{
		if (((*str) >= '0') && ((*str) <= '9'))
		{
			value <<= 4;
			value |= (*str) - '0';
		}
		else if (((*str) >= 'A') && ((*str) <= 'F'))
		{
			value <<= 4;
			value |= ((*str) - 'A') + 0xA;
		}
		else if (((*str) >= 'a') && ((*str) <= 'f'))
		{
			value <<= 4;
			value |= ((*str) - 'a') + 0xA;
		}
		else
			break;
		str++;
	}

	return (value);
}

/****************************************/

int    serial_read_data(uint8_t *buffer, int size)
{
    int			rsize = 0;
    SER_RESULT	ret;

    // While we have data on serial line
    while (size)
	{
		ret = Serial_ReadChar(&SerialModule);
		if (ret == SER_OK)
		{
			*buffer++ = (uint8_t)SerialModule.data;
			size--;
			rsize++;
		}
		else if (ret == SER_ERROR)
			break;
	}
	return (rsize);
}

void	DataTransferProtocol(void)
{
	uint8_t		*data, *ptr, answer, param[1+2+1];
	uint16_t	data_size, odata_size;
	int			i;

	data = NULL;
	if (serial_read_data(param, sizeof(param)) == sizeof(param))
	{
		if (param[1+2] == CRC8_Compute(param, 1+2))
			answer = 'G';	// Good ACK
		else
			answer = 'B';	// Bad command

		odata_size = data_size = (param[1] << 8) | param[2];

		// Send ACK
		if (Serial_SendBuf(&SerialModule, &answer, 1) != SER_OK)
			goto pcdrvout;

		if (answer == 'G')
		{
			uint8_t		*odata, crc8;
			uint16_t	lsize;

			ptr = data = malloc(data_size);

			// Read data
			if ((param[0] == 'O') || (param[0] == 'B') || (param[0] == 's') || (param[0] == 'C') || (param[0] == 'W') || (param[0] == 'p'))
			{
				while (data_size)
				{
					lsize = (data_size > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : data_size;
					odata = ptr;
					if (serial_read_data(ptr, lsize) != lsize)
						goto pcdrvout;
					ptr += lsize;
					// Check CRC
					if (serial_read_data(&crc8, 1) == 1)
					{
						if (crc8 == CRC8_Compute(odata, lsize))
						{
							answer = 'N';	// next
							data_size -= lsize;
						}
						else
						{
							answer = 'A';	// again
							ptr = odata;
						}
					}
					else
						goto pcdrvout;
                    if (Serial_SendBuf(&SerialModule, &answer, 1) != SER_OK)
						goto pcdrvout;
				}
			}

			switch (param[0])	// Process command
			{
				case 'O':	// Open file
					if (!PCdrvFD)
					{
						if (lastFileName)
							PCdrvFD = fopen(lastFileName, "rb");
						else
							PCdrvFD = fopen(data, "rb");
					}
				break;

				case 'C':	// Create file
					if (!PCdrvFD)
					{
						PCdrvFD = fopen(data, "wb+");
						if (lastFileName)	lastFileName = NULL;
						lastFileName = strdup(data);
					}
				break;

				case 'B':	// Browse file
					if (!PCdrvFD)
					{
						char	filename[MAX_PATH];
						int		dsize = strlen(data);
						char	*data2 = malloc(dsize + 6);

						strcpy(data2, data);
						memcpy(&data2[dsize], "\0*.*\0\0", 6);
						if (DoDialogOpenFile(DialogHwnd, data2, filename, sizeof(filename)))
							PCdrvFD = fopen(filename, "rb");
						free(data2);
						if (lastFileName)	lastFileName = NULL;
						lastFileName = strdup(filename);
					}
				break;

				case 'P':	// Get Position
					if (PCdrvFD)
					{
						int idata = ftell(PCdrvFD);
						data[0] = idata & 0xFF;
						data[1] = (idata >> 8) & 0xFF;
						data[2] = (idata >> 16) & 0xFF;
						data[3] = (idata >> 24) & 0xFF;
						goto send_data;
					}
				break;

				case 'S':	// Get Size
					if (PCdrvFD)
					{
						int idata, isave = ftell(PCdrvFD);
						isave = fseek(PCdrvFD, 0, SEEK_END);
						idata = ftell(PCdrvFD);
						fseek(PCdrvFD, isave, SEEK_SET);
						data[0] = idata & 0xFF;
						data[1] = (idata >> 8) & 0xFF;
						data[2] = (idata >> 16) & 0xFF;
						data[3] = (idata >> 24) & 0xFF;
						goto send_data;
					}
				break;

				case 's':	// Seek
					if (PCdrvFD)
					{
						int idata = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];

						fseek(PCdrvFD, idata, SEEK_SET);
					}
				break;

				case 'R':	// Read file
					if (PCdrvFD)
					{
						fread(data, 1, data_size, PCdrvFD);
send_data:
						// Send Data
						while (data_size)
						{
							lsize = (data_size > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : data_size;
							odata = ptr;
							if (Serial_SendBuf(&SerialModule, ptr, lsize) != SER_OK)
								goto pcdrvout;
							ptr += lsize;
							// Check CRC
							if (serial_read_data(&crc8, 1) == 1)
							{
								if (crc8 == CRC8_Compute(odata, lsize))
								{
									answer = 'N';	// next
									data_size -= lsize;
								}
								else
								{
									answer = 'A';	// again
									ptr = odata;
								}
							}
							else
								goto pcdrvout;
							if (Serial_SendBuf(&SerialModule, &answer, 1) != SER_OK)
								goto pcdrvout;
						}
					}
				break;

				case 'p':	// Progress Bar
					SetProgressBarPosition(DialogHwnd, IDC_PROGRESS, data[0]);
				break;

				case 'W':	// Write file
					if (PCdrvFD)
						fwrite(data, 1, odata_size, PCdrvFD);
				break;

				case 'c':	// Close file
					if (PCdrvFD)
					{
						fclose(PCdrvFD);
						PCdrvFD = NULL;
					}
				break;
			}
		}
	}
pcdrvout:
	if (data)
		free(data);
}
