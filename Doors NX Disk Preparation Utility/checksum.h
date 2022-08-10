#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

unsigned int crc32checksum(int arrLength, FILE* filePointer, int fileStartPos);

#endif