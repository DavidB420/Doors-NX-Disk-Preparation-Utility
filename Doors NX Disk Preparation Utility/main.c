#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <stdbool.h>
#include "diskio.h"

void zeroFillBuffer(char* ptr, int length);
void writeDwordToFile(UINT32 dword, int position,FILE *filePointer);

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

	while (errorCode = fopen_s(&osImage, userInputBuffer, "r+") != 0)
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
	ULONGLONG diskSizes[24] = { 0 };

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
				diskSizes[fdisk.DeviceNumber] = diskSizeGeo.Cylinders.QuadPart * (ULONG)diskSizeGeo.TracksPerCylinder * (ULONG)diskSizeGeo.SectorsPerTrack * (ULONG)diskSizeGeo.BytesPerSector;
				printf("%i			%I64d\n", fdisk.DeviceNumber,diskSizes[fdisk.DeviceNumber]);
				driveNumbers[driveNumberPointer] = fdisk.DeviceNumber;
				driveNumberPointer++;
			}
			CloseHandle(driveHandler);
		}
	}

	//Ask the user for their chosen drive
	zeroFillBuffer(&userInputBuffer[0], _MAX_PATH);
	printf("\nPlease select a drive with its drive number: ");
	scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);

	while (!checkForDuplicates(&driveNumbers, atoi(&userInputBuffer[0])))
	{
		printf("ERROR: Invalid disk choice!\nPlease try again: ");
		scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);
	}

	ULONGLONG selectedDiskSize = diskSizes[atoi(&userInputBuffer[0])];

	printf("\nWARNING! ALL DATA ON THIS DISK WILL BE LOST! Type ""yes"" to continue: ");

	while (1)
	{
		scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);

		if (strcmp(userInputBuffer,"yes") == 0)
		{
			break;
		}
		else
		{
			printf("Program terminated. User did not user enter ""yes""");
			return 0;
		}
	}

	fseek(osImage, 0x1c2, SEEK_SET);
	fputc(0xee,osImage);

	writeDwordToFile(0xffffffff, 0x1ca, osImage);

	writeDwordToFile(0xffffff, 0x1c3, osImage);

	writeDwordToFile(1, 0x1c6, osImage);



	return 0;
}

void zeroFillBuffer(char *ptr, int length)
{
	for (int i = 0; i < length; i++)
	{
		ptr[i] = 0;
	}
}

void writeDwordToFile(UINT32 dword, int position, FILE* filePointer)
{
	UINT32 tempDword = dword;

	fseek(filePointer, position, SEEK_SET);

	for (int i = 0; i < 4; i++)
	{
		fputc(tempDword & 0xff, filePointer);
		tempDword = tempDword >> 8;
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