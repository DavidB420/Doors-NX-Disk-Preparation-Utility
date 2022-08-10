#include "diskio.h"

void diskReadSector(int drivenum, int sectortoRead)
{
	WCHAR driveBuffer[] = L"\\\\.\\PhysicalDrive0";
	wchar_t logicalDriveAddr[_MAX_PATH] = { 0 };
	BYTE buff[512] = {0};
	BOOL readSector;
	DWORD bytesRead = 0;

	driveBuffer[17] = drivenum + 0x30;

	GetLogicalDriveStringsW(_MAX_PATH, &logicalDriveAddr);

	HANDLE driveHandler = CreateFile(driveBuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	SetFilePointer(driveHandler, sectortoRead * 512, 0, FILE_BEGIN);

	while(!(readSector = ReadFile(driveHandler, buff, 512, &bytesRead, NULL)));

	for (int i = 0; i < 512; i++)
	{
		printf("%c", buff[i]);
	}

	CloseHandle(driveHandler);
}

void diskWriteSector()
{
}
