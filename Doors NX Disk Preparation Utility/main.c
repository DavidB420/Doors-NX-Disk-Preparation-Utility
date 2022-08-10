#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

void zeroFillBuffer(char* ptr, int length);

int main(int argc,char** argv)
{
	//Declare variables
	FILE* osImage;
	char userInputBuffer[_MAX_PATH];
	errno_t errorCode;
	DWORD val;
	WCHAR driveBuffer[_MAX_PATH] = {0};

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

	printf("\nDisk #			Size (bytes)\n");
	printf("------			-------------------------\n");

	for (int i = 2; i < 31; i++)
	{
		if ((val & (1 << i)) >> i == 1)
		{
			printf("%i\n",i-2);
		}
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
