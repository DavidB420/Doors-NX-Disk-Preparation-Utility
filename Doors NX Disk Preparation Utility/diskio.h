#ifndef DISKIO_H
#define DISKIO_H

#include <windows.h>
#include <winioctl.h>

void diskReadSector(int drivenum);
void diskWriteSector();

#endif