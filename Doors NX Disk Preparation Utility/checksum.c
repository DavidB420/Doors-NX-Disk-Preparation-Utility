#include "checksum.h"

unsigned int crc32checksum(int arrLength, FILE *filePointer, int fileStartPos)
{
	const int arrSize = arrLength;
	unsigned long long int value = 0xFFFFFFFF;
	unsigned long long int crc32polynomial = 0x1DB710641;
	BYTE buff[0xffff] = { 0 };
	BYTE tempByte;
	int currentPos = fileStartPos;

	fseek(filePointer, currentPos, SEEK_SET);
	for (int i = 0; i < arrLength; i++)
	{
		tempByte = fgetc(filePointer);
		buff[i] = tempByte;
	}

	for (int x = 0; x < arrLength; x++)
	{
		value = value ^ buff[x];

		for (int i = 0; i < 8; i++)
		{
			if (value & ((unsigned long long int)1 << i))
			{
				value = value ^ crc32polynomial;
			}
			crc32polynomial = crc32polynomial << 1;
		}

		value = value >> 8;

		crc32polynomial = 0x1DB710641;
	}
	
	value = (value ^ (0xFFFFFFFF));


	return value;
}