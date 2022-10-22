//
// Created by nuclaer on 2/11/22.
//

#include "drivers/filesystems/fat32.h"
#include "ntios.h"


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
	// We need to flush to the disk here
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

List<FSObject>* Fat32Driver::readDir(uint32_t cluster) {
	// if the cluster number is zero, assume we're trying to read from the root directory.
	if(cluster == 0)
		cluster = this->root_cluster;

	// Get the cluster chain of the directory. This contains the directory entries themselves.
	// We will implement GetClusterChain later.
	uint64_t numclus;
	List<uint32_t>* clusters = this->GetClusterChain(node, &numclus);

	// try and read each cluster into a contiguous buffer.

	// this is the total number of bytes the directory is (in terms of on-disk dirents)
	uint64_t dirsize = numclus * this->SectorsPerCluster * 512;

	// allocate a buffer, rounding up the size to a page.
	uint64_t buf = AllocateBuffer(numclus * this->SectorsPerCluster * 512);
	uint64_t obuf = buf;

	assert(clusters);
	foreach(uint32_t cluster in clusters)
	{
		// read "SectorsPerCluster * 512" bytes at the LBA "ClusterToLBA(cluster)" into buf.
		ReadFromDisk(ClusterToLBA(cluster), buf, this->SectorsPerCluster * 512);

		// increment buf, so we don't trash its contents.
		buf += this->SectorsPerCluster * 512;
	}
	buf = obuf;

	// we now have the complete data of the directory.
	// ... to be implemented
}

void Traverse(const char* path) {
	// or whatever here, basically split the paths into its components.
	List<const char*> pathparts = split(path, '/');

	// get the root directory.
	List<FSObject>* dirs = ReadDir(this->RootDirectoryCluster);

	// iterate through it.
	foreach(FSObject fso in dirs)
	{
		// again, FSObject should be a VFS node of some kind, with FS specific information stored in a pointer within.
		// I'll just assume it has a member called 'cluster' at this point.

		// TODO
	}
}

List<uint32_t>* GetClusterChain(uint32_t firstcluster, uint64_t* numclus)
{
	// setup some stuff.
	uint32_t Cluster = firstcluster;
	uint32_t cchain = 0;
	List<uint32_t>* ret = new List<uint32_t>();

	// here we assume your 'ReadFromDisk' only does 512 bytes at a time or something.
	auto buf = AllocateBuffer(512);
	auto obuf = buf;
	do
	{
		// these formulas are gotten from the 'fatgen103' document.
		// 'FatSector' is the LBA of the disk at which the FAT entry you want is located.
		// 'FatOffset' is the offset in bytes from the beginning of 'FatSector' to the cluster.
		uint32_t FatSector = (uint32_t) this->partition->GetStartLBA() + this->ReservedSectors + ((Cluster * 4) / 512);
		uint32_t FatOffset = (Cluster * 4) % 512;

		// read 512 bytes into buf.
		ReadFromDisk(FatSector, buf, 512);

		// cast to an array so we get an easier time.
		uint8_t* clusterchain = (uint8_t*) buf;

		// using FatOffset, we just index into the array to get the value we want.
		cchain = *((uint32_t*)&clusterchain[FatOffset]) & 0x0FFFFFFF;

		// the value of 'Cluster' will change by the next iteration.
		// Because we're nice people, we need to include the first cluster in the list of clusters we return.
		ret->push_back(Cluster);

		// since cchain is the next cluster in the list, we just modify the things, shouldn't be too hard to grasp.
		Cluster = cchain;

		// numclus tells the caller how many clusters are in the chain.
		(*numclus)++;

	} while((cchain != 0) && !((cchain & 0x0FFFFFFF) >= 0x0FFFFFF8));

	// check that there's more entries in the chain, if not break the loop.
	// the reason for using a do-while should be obvious.

	return ret;
}

bool Fat32Driver::readSector(uint64_t sector, uint8_t* dst) {
	if (sector >= length) return false;
	return disk->readSector(start + sector, dst);
}

bool Fat32Driver::writeSector(uint64_t sector, uint8_t* src) {
	if (sector >= length) return false;
	return disk->writeSector(start + sector, src);
}

