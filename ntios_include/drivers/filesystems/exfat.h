//
// Created by nuclaer on 2/10/22.
//

#ifndef NTIOS_2021_EXFAT_H
#define NTIOS_2021_EXFAT_H

#include "drivers.h"
#include "blockdevice.h"


class ExFatDriver: public FileSystemDevice {
public:
	ExFatDriver(BlockDevice* disk, uint32_t start, uint32_t length);

	const char* getName() { return "ExFAT"; }

	int mount();
	void unmount();
	NTIOSFile* open(const char *filename, uint8_t mode = NTIOS_READ);
	bool exists(const char *filepath);
	bool mkdir(const char *filepath);
	bool remove(const char *filepath);
	bool rmdir(const char *filepath);

private:
	bool readSector(uint64_t sector, uint8_t* dst);
	bool writeSector(uint64_t sector, uint8_t* src);
	uint64_t start, end, length;
	BlockDevice* disk;
};

#endif //NTIOS_2021_EXFAT_H
