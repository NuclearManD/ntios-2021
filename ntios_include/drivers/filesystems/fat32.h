//
// Created by nuclaer on 2/11/22.
//

#ifndef NTIOS_2021_FAT32_H
#define NTIOS_2021_FAT32_H

#include "drivers.h"
#include "blockdevice.h"


class Fat32Driver: public FileSystemDevice {
public:
	Fat32Driver(BlockDevice* disk, uint32_t start, uint32_t length);

	const char* getName() { return "FAT32"; }

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
	char label[12];
	BlockDevice* disk;

	uint16_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t fat_count;
	uint16_t num_root_entries;
	uint32_t total_sectors;

	uint32_t sectors_per_fat;
	uint32_t root_cluster;
	uint16_t fsinfo_sector;
};

#endif //NTIOS_2021_FAT32_H
