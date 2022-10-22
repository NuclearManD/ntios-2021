//
// Created by nuclaer on 2/11/22.
//

#include "drivers/disks.h"
#include "drivers/filesystems/exfat.h"
#include "drivers/filesystems/fat32.h"
#include "stdint.h"
#include "ntios.h"

Partition::Partition(const char* typeName, uint64_t start, uint64_t partitionLength, bool isHealthy, FileSystemDevice* fs) {
	firstSector = start;
	lastSector = start + partitionLength - 1;
	length = partitionLength;
	type_name = typeName;
	is_healthy = isHealthy;
	this->file_system = fs;
}



typedef struct mbrPartitionEntry_s {
	uint8_t status;
	uint8_t chs_start[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint8_t lba_start[4];
	uint8_t num_sectors[4];
} mbrPartitionEntry_t;

typedef struct sector0_s {
	uint8_t jump[3];
	uint8_t bootcode[443];
	mbrPartitionEntry_t partitions[4];
	uint8_t boot_signature[2];
} sector0_t;


int PartitionedDisk::begin(BlockDevice* disk) {
	if (!disk->begin())
		return ERR_HW_NOT_OPERATIONAL;

	sector0_t sector0;
	if (!disk->readSector(0, (uint8_t*)&sector0))
		return ERR_READ_FAILED;

	int n = 0;
	Partition mbrPartitionsTmp[4];
	record_type = RECORD_TYPE_MBR;
	for (int i = 0; i < 4; i++) {
		uint8_t partition_type = sector0.partitions[i].type;
		uint32_t lba_start = LI_TO_UINT32(sector0.partitions[i].lba_start);
		uint32_t num_sectors = LI_TO_UINT32(sector0.partitions[i].num_sectors);

		if (partition_type == 0xee || partition_type == 0xef) {
			if (i != 0) {
				// GPT partition type
				record_type = RECORD_TYPE_GPT;
			} else {
				mbrPartitionsTmp[n++] = Partition("Invalid GPT entry", lba_start, num_sectors, false, nullptr);
			}
		} else if (partition_type == 0x0b || partition_type == 0x0c) {
			// FAT32
			Fat32Driver* fs = new Fat32Driver(disk, lba_start, num_sectors);
			bool isHealthy = fs->mount() == 0;
			mbrPartitionsTmp[n++] = Partition("FAT32", lba_start, num_sectors, isHealthy, fs);
		} else if (partition_type == 0x07) {
			// ExFAT
			ExFatDriver* fs = new ExFatDriver(disk, lba_start, num_sectors);
			bool isHealthy = fs->mount() == 0;
			mbrPartitionsTmp[n++] = Partition("ExFAT", lba_start, num_sectors, isHealthy, fs);
			n++;
		} else if (partition_type == 0x43 || partition_type == 0x83) {
			mbrPartitionsTmp[n++] = Partition("Linux", lba_start, num_sectors, false, nullptr);
		} else if (partition_type == 0x93) {
			mbrPartitionsTmp[n++] = Partition("Linux (hidden)", lba_start, num_sectors, false, nullptr);
		} else if (partition_type == 0x04 || partition_type == 0x06 || partition_type == 0x0e) {
			mbrPartitionsTmp[n++] = Partition("FAT16", lba_start, num_sectors, false, nullptr);
		} else if (partition_type == 0x41 || partition_type == 0x81) {
			mbrPartitionsTmp[n++] = Partition("Minix", lba_start, num_sectors, false, nullptr);
		} else if (partition_type == 0x42 || partition_type == 0x82) {
			mbrPartitionsTmp[n++] = Partition("Linux Swap", lba_start, num_sectors, false, nullptr);
		} else if (partition_type != 0x00) {
			mbrPartitionsTmp[n++] = Partition("Unknown", lba_start, num_sectors, false, nullptr);
		}
	}
	num_partitions = n;
	partitions = (Partition*)malloc(sizeof(Partition)* n);
	memcpy(partitions, mbrPartitionsTmp, sizeof(Partition)* n);
	return 0;
}

const char* PartitionedDisk::diskRecordTypeAsStr() {
	switch (record_type) {
		case RECORD_TYPE_INVALID:
			return "invalid";
		case RECORD_TYPE_NONE:
			return "none";
		case RECORD_TYPE_MBR:
			return "MBR";
		case RECORD_TYPE_GPT:
			return "GPT";
		default:
			return "unknown";
	}
}

PartitionedDisk::~PartitionedDisk() {
	if (partitions != nullptr) {
		for (uint32_t i = 0; i < num_partitions; i++) {
			FileSystemDevice* fs = partitions[i].getFilesystem();
			if (fs != nullptr) {
				fs->unmount();
				delete fs;
			}
		}
		free(partitions);
	}
}
