//
// Created by nuclaer on 2/11/22.
//

#include "drivers/filesystems/fat32.h"


Fat32Driver::Fat32Driver(BlockDevice* disk, uint32_t start, uint32_t length) {
	this->disk = disk;
	this->start = start;
	this->end = start + length;
	this->length = length;
}

int Fat32Driver::mount() {
	if (!disk->begin())
		return ERR_HW_NOT_OPERATIONAL;

	unsigned char sector0[512];
	if (!readSector(0, sector0))
		return ERR_READ_FAILED;

	if (memcmp((const char*)&sector0[0x052], "FAT32   ", 8))
		return ERR_WRONG_FS;

	// Copy disk label
	memcpy(label, (const char*)&sector0[0x047], 11);
	for (int i = 10; i >= 0; i--) {
		if (label[i] != ' ') break;
		label[i] = 0;
	}
	label[11] = 0;

	/*uint8_t signature = sector0[0x042];
	if (signature != 0x28 && signature != 0x29)
		return ERR_FS_CORRUPT;*/

	sectors_per_cluster = sector0[0x0d];
	reserved_sectors = LI_TO_UINT16(&sector0[0x0e]);
	fat_count = sector0[0x10];
	num_root_entries = LI_TO_UINT16(&sector0[0x11]);
	total_sectors = LI_TO_UINT16(&sector0[0x13]);
	if (total_sectors == 0)
		total_sectors = LI_TO_UINT32(&sector0[0x20]);

	sectors_per_fat = LI_TO_UINT32(&sector0[0x24]);
	root_cluster = LI_TO_UINT32(&sector0[0x2c]);
	fsinfo_sector = LI_TO_UINT16(&sector0[0x30]);

	return 0;
}

void Fat32Driver::unmount() {

}

NTIOSFile* Fat32Driver::open(const char *filename, uint8_t mode) {
	return nullptr;
}

bool Fat32Driver::exists(const char *filepath) {
	return false;
}

bool Fat32Driver::mkdir(const char *filepath) {
	return false;
}

bool Fat32Driver::remove(const char *filepath) {
	return false;
}

bool Fat32Driver::rmdir(const char *filepath) {
	return false;
}

bool Fat32Driver::readSector(uint64_t sector, uint8_t* dst) {
	if (sector >= length) return false;
	return disk->readSector(start + sector, dst);
}

bool Fat32Driver::writeSector(uint64_t sector, uint8_t* src) {
	if (sector >= length) return false;
	return disk->writeSector(start + sector, src);
}

