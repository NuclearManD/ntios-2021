//
// Created by nuclaer on 2/11/22.
//

#ifndef NTIOS_2021_DISKS_H
#define NTIOS_2021_DISKS_H

#include "blockdevice.h"

#define RECORD_TYPE_INVALID -1
#define RECORD_TYPE_NONE 0
#define RECORD_TYPE_MBR 1
#define RECORD_TYPE_GPT 2


class Partition {
public:
	Partition() { type_name = "?"; }
	Partition(const char* typeName, uint64_t start, uint64_t partitionLength, bool isHealthy, FileSystemDevice* fs);

	// Size and position related utilities
	uint64_t getLength() { return length; }
	uint64_t getStart() { return firstSector; }
	uint64_t getEnd() { return lastSector; }
	uint64_t getSizeBytes() { return length * 512; }
	double getSizeMiB() { return length / 2048.0; }
	double getSizeGiB() { return length / 2097152.0; }

	// Other info
	bool isHealthy() { return is_healthy; }
	FileSystemDevice* getFilesystem() { return file_system; }
	const char* getTypeAsStr() { return type_name; }
private:
	uint64_t firstSector, lastSector, length;
	bool is_healthy;
	const char* type_name;
	FileSystemDevice* file_system;
};



class PartitionedDisk {
public:
	int begin(BlockDevice* device);

	int diskRecordType() {
		return record_type;
	}

	const char* diskRecordTypeAsStr();

	uint32_t numPartitions() {
		return num_partitions;
	}
	Partition& getPartition(uint32_t index) {
		return partitions[index];
	}

	~PartitionedDisk();

private:
	int record_type = -2;
	uint32_t num_partitions = 0;
	Partition* partitions = nullptr;
};

#endif //NTIOS_2021_DISKS_H
