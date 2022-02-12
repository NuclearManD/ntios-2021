//
// Created by nuclaer on 2/10/22.
//

#include "drivers/filesystems/exfat.h"


ExFatDriver::ExFatDriver(BlockDevice* disk, uint32_t start, uint32_t length) {
	this->disk = disk;
	this->start = start;
	this->end = start + length;
	this->length = length;
}

int ExFatDriver::mount() {
	if (!disk->begin())
		return ERR_HW_NOT_OPERATIONAL;

	unsigned char sector0[512];
	if (!readSector(0, sector0))
		return ERR_READ_FAILED;

	if (strcmp((const char*)&sector0[3], "EXFAT   "))
		return ERR_WRONG_FS;

	return 0;
}

void ExFatDriver::unmount() {

}

NTIOSFile* ExFatDriver::open(const char *filename, uint8_t mode) {
	return nullptr;
}

bool ExFatDriver::exists(const char *filepath) {
	return false;
}

bool ExFatDriver::mkdir(const char *filepath) {
	return false;
}

bool ExFatDriver::remove(const char *filepath) {
	return false;
}

bool ExFatDriver::rmdir(const char *filepath) {
	return false;
}

bool ExFatDriver::readSector(uint64_t sector, uint8_t* dst) {
	if (sector >= length) return false;
	return disk->readSector(start + sector, dst);
}

bool ExFatDriver::writeSector(uint64_t sector, uint8_t* src) {
	if (sector >= length) return false;
	return disk->writeSector(start + sector, src);
}

