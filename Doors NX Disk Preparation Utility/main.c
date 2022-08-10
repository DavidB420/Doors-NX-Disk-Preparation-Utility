#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <stdbool.h>
#include <math.h>
#include "diskio.h"

void zeroFillBuffer(char* ptr, int length);
void writeDwordToFile(UINT32 dword, int position,FILE *filePointer);
void saveToSector(int pos, FILE* filePointer, int sectorNum, int driveNum, int numOfSectorsToSave);

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

	while (errorCode = fopen_s(&osImage, userInputBuffer, "rb+") != 0)
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
	DISK_GEOMETRY_EX diskSizeGeo = { 0 };
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
				driveHandler = CreateFile(driveBuffer, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				DeviceIoControl(driveHandler, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskSizeGeo, sizeof(diskSizeGeo), &sizeOfReturn, NULL);
				diskSizes[fdisk.DeviceNumber] = diskSizeGeo.DiskSize.QuadPart;
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

	while (!checkForDuplicates(&driveNumbers, atoi(&userInputBuffer[0])) || atoi(&userInputBuffer[0]) <= 2)
	{
		printf("ERROR: Invalid disk choice!\nPlease try again: ");
		scanf_s("%[^\n]%*c", userInputBuffer, _MAX_PATH);
	}

	int diskNum = atoi(&userInputBuffer[0]);
	ULONGLONG selectedDiskSize = diskSizes[diskNum];

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

	writeDwordToFile((selectedDiskSize / 512) - 34, 0x230, osImage);

	writeDwordToFile((selectedDiskSize / 512) - 34, 0x428, osImage);

	writeDwordToFile((selectedDiskSize / 512) - 1, 0x220, osImage);

	//Determine if FAT12, 16, or 32 should be used, depending on the file size
	if (selectedDiskSize <= 16000000)
	{
		fseek(osImage, 0x440d, SEEK_SET);
		fputc(8, osImage);

		fseek(osImage, 0x4413, SEEK_SET);
		fputc((selectedDiskSize / 512) & 0xff, osImage);
		fseek(osImage, 0x4414, SEEK_SET);
		fputc(((selectedDiskSize / 512) & 0xff) >> 8, osImage);

		fseek(osImage, 0x4416, SEEK_SET);
		fputc((int)ceil(((selectedDiskSize / 512) * 1.5 / 512))  & 0xff, osImage);
		fseek(osImage, 0x4417, SEEK_SET);
		fputc((int)ceil(((selectedDiskSize / 512) * 1.5 / 512)) >> 8, osImage);
	}
	else
	{
		fseek(osImage, 0x440d, SEEK_SET);
		fputc(8, osImage);

		fseek(osImage, 0x4413, SEEK_SET);
		fputc((16000000 / 512) & 0xff, osImage);
		fseek(osImage, 0x4414, SEEK_SET);
		fputc(((16000000 / 512) & 0xff) >> 8, osImage);

		fseek(osImage, 0x4416, SEEK_SET);
		fputc((int)ceil(((16000000 / 512) * 1.5 / 512)) & 0xff, osImage);
		fseek(osImage, 0x4417, SEEK_SET);
		fputc((int)ceil(((16000000 / 512) * 1.5 / 512)) >> 8, osImage);
	}

	saveToSector(0, osImage,0, diskNum,33);

	saveToSector(0x400, osImage, (selectedDiskSize / 512) - 33, diskNum, 31);
	saveToSector(0x400, osImage, (selectedDiskSize / 512) - 33, diskNum, 31);
	saveToSector(0x200, osImage, (selectedDiskSize / 512) - 1, diskNum, 1);
	saveToSector(0x4400, osImage, 34, diskNum, 1);

	printf("Disk Write Complete!\n");
	system("PAUSE");

	return 0;
}

void saveToSector(int pos, FILE * filePointer, int sectorNum, int driveNum, int numOfSectorsToSave)
{
	int currentPos = pos;
	int currentSectorNum = sectorNum;
	int currentNumOfSectorsToSave = numOfSectorsToSave + sectorNum;
	BYTE buff[512] = { 0 };
	BYTE tempByte;

	for (currentSectorNum; currentSectorNum < currentNumOfSectorsToSave; currentSectorNum++)
	{
		fseek(filePointer, currentPos, SEEK_SET);

		for (int i = 0; i < 512; i++)
		{
			tempByte = fgetc(filePointer);
			buff[i] = tempByte;
		}

		diskWriteSector(driveNum, currentSectorNum, &buff);
		currentPos += 512;
	}
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