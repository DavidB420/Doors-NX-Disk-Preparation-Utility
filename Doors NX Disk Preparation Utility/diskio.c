#include "diskio.h"

void diskReadSector(int drivenum, int sectortoRead)
{
	WCHAR driveBuffer[_MAX_PATH] = { 0 };
	wchar_t logicalDriveAddr[_MAX_PATH] = { 0 };
	char buff[512] = {0};
	BOOL readSector;

	GetLogicalDriveStringsW(_MAX_PATH, &logicalDriveAddr);
	GetVolumeNameForVolumeMountPointW(&logicalDriveAddr[(drivenum) * 4], &driveBuffer, _MAX_PATH);
	driveBuffer[48] = 0;

	HANDLE driveHandler = CreateFileW(driveBuffer, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	SetFilePointer(driveHandler, sectortoRead * 512, 0, FILE_BEGIN);

	while(readSector = ReadFile(driveHandler, &buff, 512, 0, FILE_BEGIN));

	CloseHandle(driveHandler);
}

void diskWriteSector()
{
}
