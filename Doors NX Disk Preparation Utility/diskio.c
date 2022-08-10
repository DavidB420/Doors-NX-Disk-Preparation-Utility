#include "diskio.h"

void diskReadSector(int drivenum, int sectortoRead, BYTE* buffptr)
{
	WCHAR driveBuffer[] = L"\\\\.\\PhysicalDrive0";
	BYTE buff[512] = {0};
	BOOL readSector;
	DWORD bytesRead = 0;

	driveBuffer[17] = drivenum + 0x30;

	HANDLE driveHandler = CreateFile(driveBuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	SetFilePointer(driveHandler, sectortoRead * 512, 0, FILE_BEGIN);

	while(!(readSector = ReadFile(driveHandler, buffptr, 512, &bytesRead, NULL)));

	for (int i = 0; i < 512; i++)
	{
		printf("%c", buffptr[i]);
	}

	CloseHandle(driveHandler);
}

void diskWriteSector(int drivenum, int sectortoWrite, BYTE *buffptr)
{
	WCHAR driveBuffer[] = L"\\\\.\\PhysicalDrive0";
	BOOL writeSector;
	DWORD bytesWritten = 0;
	wchar_t logicalDriveAddr[_MAX_PATH] = { 0 };
	DWORD val = GetLogicalDrives();
	WCHAR driveBuffer2[_MAX_PATH] = { 0 };
	STORAGE_DEVICE_NUMBER fdisk = { 0 };

	GetLogicalDriveStringsW(_MAX_PATH, &logicalDriveAddr);
	for (int i = 2; i < 31; i++)
	{
		if ((val & (1 << i)) >> i == 1)
		{
			GetVolumeNameForVolumeMountPointW(&logicalDriveAddr[(i - 2) * 4], &driveBuffer2, _MAX_PATH);
			driveBuffer2[48] = 0;
			HANDLE driveHandler2 = CreateFile(driveBuffer2, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			DeviceIoControl(driveHandler2, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &fdisk, sizeof(fdisk), &bytesWritten, NULL);
			if (fdisk.DeviceNumber == drivenum)
			{
				//Dismount and lock volume
				DeviceIoControl(driveHandler2, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bytesWritten, NULL);
				DeviceIoControl(driveHandler2, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesWritten, NULL);
			}
			CloseHandle(driveHandler2);
		}
	}
	driveBuffer[17] = drivenum + 0x30;

	HANDLE driveHandler = CreateFile(driveBuffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	SetFilePointer(driveHandler, (LONG)(sectortoWrite * 512), 0, FILE_BEGIN);

	writeSector = WriteFile(driveHandler, buffptr, 512, &bytesWritten, NULL);

	CloseHandle(driveHandler);
}
