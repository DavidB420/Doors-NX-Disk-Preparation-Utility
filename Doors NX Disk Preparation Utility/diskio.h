#ifndef DISKIO_H
#define DISKIO_H

#include <windows.h>
#include <winioctl.h>

void diskReadSector(int drivenum, int sectortoRead);
void diskWriteSector();

#endif