#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <stdbool.h>

void zeroFillBuffer(char* ptr, int length);

bool checkForDuplicates(DWORD* ptr,DWORD value);

int main(int argc, char** argv)
{
	//Declare variables
	FILE* osImage;
	char userInputBuffer[_MAX_PATH];
	errno_t errorCode;
	DWORD val;
	DWORD sizeOfReturn;
	WCHAR driveBuffer[_MAX_PATH] = { 0 };
	wchar_t logicalDriveAddr[_MAX_PATH] = {0};
	DWORD driveNumbers[32] = {0xFFFFFFFF};
	int driveNumberPointer = 0;
	int inttemp;

	//Initialize buffer by zero filling it
	zeroFillBuffer(&userInputBuffer[0], _MAX_PATH);

	//Load OS image
	printf("Doors NX GPT Disk Preparation Utility\nCopyright (C) 2022 David Badiei\n\nEnter Doors NX image name here: ");
	scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);

	while (errorCode = fopen_s(&osImage, userInputBuffer, "r") != 0)
	{
		printf("ERROR: Cannot locate Doors NX image!\nPlease try again: ");
		scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);
	}

	//Get and display available drives
	val = GetLogicalDrives();
	GetLogicalDriveStringsW(_MAX_PATH, &logicalDriveAddr);

	printf("\nDisk #			Size (bytes)\n");
	printf("------			-------------------------\n");

	

	for (int i = 0; i < 31; i++)
	{
		driveNumbers[i] = 0xFFFFFFFF;
	}
	
	STORAGE_DEVICE_NUMBER fdisk = { 0 };
	DISK_GEOMETRY diskSizeGeo = { 0 };
	ULONGLONG diskSize = 0;

	for (int i = 2; i < 31; i++)
	{
		if ((val & (1 << i)) >> i == 1)
		{
			GetVolumeNameForVolumeMountPointW(&logicalDriveAddr[(i-2) * 4], &driveBuffer, _MAX_PATH);
			driveBuffer[48] = 0;
			HANDLE driveHandler = CreateFile(driveBuffer, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			DeviceIoControl(driveHandler, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &fdisk, sizeof(fdisk), &sizeOfReturn,NULL);
			if (!checkForDuplicates(&driveNumbers, fdisk.DeviceNumber))
			{
				DeviceIoControl(driveHandler, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskSizeGeo, sizeof(diskSizeGeo), &sizeOfReturn, NULL);
				diskSize = diskSizeGeo.Cylinders.QuadPart * (ULONG)diskSizeGeo.TracksPerCylinder * (ULONG)diskSizeGeo.SectorsPerTrack * (ULONG)diskSizeGeo.BytesPerSector;
				printf("%i			%I64d\n", fdisk.DeviceNumber,diskSize);
				driveNumbers[driveNumberPointer] = fdisk.DeviceNumber;
				driveNumberPointer++;
			}
			CloseHandle(driveHandler);
		}
	}

	zeroFillBuffer(&userInputBuffer[0], _MAX_PATH);
	printf("\nPlease select a drive with its drive number: ");
	scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);

	while (!checkForDuplicates(&driveNumbers, atoi(&userInputBuffer[0])))
	{
		printf("ERROR: Invalid disk choice!\nPlease try again: ");
		scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);
	}

	return 0;
}

void zeroFillBuffer(char *ptr, int length)
{
	for (int i = 0; i < length; i++)
	{
		ptr[i] = 0;
	}
}

bool checkForDuplicates(DWORD* ptr, DWORD value)
{
	for (int i = 0; i < 31; i++)
	{
		if (ptr[i] == value)
		{
			return true;
		}
	}
	return false;
}
