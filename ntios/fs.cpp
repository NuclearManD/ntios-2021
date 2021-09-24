
#include "drivers.h"
#include "ntios.h"

#define MAX_MOUNT_DIR_LEN 64
#define MAX_MOUNT_POINTS 8

char mount_names[MAX_MOUNT_POINTS][MAX_MOUNT_DIR_LEN + 1];
FileSystemDevice* mount_fs[MAX_MOUNT_POINTS];
int mount_id[MAX_MOUNT_POINTS];
int num_mounted = 0;

int getFilesystemByDev(int devid, FileSystemDevice** fs) {
	Device* dev = get_device(devid);

	if (dev == NULL)
		return ERR_NO_DEVICE;
	if ((dev->getType() & 0xF00) != DEV_TYPE_FILESYSTEM)
		return ERR_NOT_FS;
	*fs = (FileSystemDevice*)dev;
	return 0;
}

int mount(int dev, const char* dir) {

	if (strlen(dir) >= MAX_MOUNT_DIR_LEN)
		return -1000;

	if (num_mounted >= MAX_MOUNT_POINTS)
		return -1001;

	FileSystemDevice* fs;
	int ret;

	if (0 > (ret = getFilesystemByDev(dev, &fs)))
		return ret;

	fs->mount();
	mount_fs[num_mounted] = fs;
	mount_id[num_mounted] = dev;
	strcpy(mount_names[num_mounted], dir);
	num_mounted++;
	return 0;
}

int unmount(int dev) {
	for (int i = 0; i < num_mounted; i++) {
		if (mount_id[i] == dev) {
			FileSystemDevice* fs = mount_fs[i];
			num_mounted--;
			mount_fs[i] = mount_fs[num_mounted];
			mount_id[i] = mount_id[num_mounted];
			strcpy((char*)mount_names[i], (const char*)mount_names[num_mounted]);
			fs->unmount();
			return 0;
		}
	}
	return ERR_NOT_MOUNTED;
}

bool str_startswith(const char* s, const char* pre) {
	while (*pre) {
		if (*pre != *s)
			return 0;
		pre++;
		s++;
	}
	return 1;
}

NTIOSFile* fsopen(const char* dir, int mode) {
	for (int i = 0; i < num_mounted; i++) {
		if (str_startswith(dir, (const char*)mount_names[i])) {
			dir += strlen((const char*)mount_names[i]);
			FileSystemDevice* fs = mount_fs[i];
			return fs->open(dir, mode);
		}
	}
	return NULL;
}

int fsmkdir(const char* dir) {
	for (int i = 0; i < num_mounted; i++) {
		if (str_startswith(dir, (const char*)mount_names[i])) {
			dir += strlen((const char*)mount_names[i]);
			FileSystemDevice* fs = mount_fs[i];
			if (fs->mkdir(dir))
				return 0;
			else
				return ERR_UNKNOWN;
		}
	}
	return ERR_NO_DIR;
}

int fsrmdir(const char* dir) {
	for (int i = 0; i < num_mounted; i++) {
		if (str_startswith(dir, (const char*)mount_names[i])) {
			dir += strlen((const char*)mount_names[i]);
			FileSystemDevice* fs = mount_fs[i];
			if (fs->rmdir(dir))
				return 0;
			else
				return ERR_UNKNOWN;
		}
	}
	return ERR_NO_DIR;
}

int fsremove(const char* dir) {
	for (int i = 0; i < num_mounted; i++) {
		if (str_startswith(dir, (const char*)mount_names[i])) {
			dir += strlen((const char*)mount_names[i]);
			FileSystemDevice* fs = mount_fs[i];
			if (fs->remove(dir))
				return 0;
			else
				return ERR_UNKNOWN;
		}
	}
	return ERR_NO_DIR;
}

int fscopy(const char* src, const char* dst) {
	NTIOSFile* srcf = fsopen(src, NTIOS_READ);
	if (!srcf) return ERR_NO_DIR;

	NTIOSFile* dstf = fsopen(dst, NTIOS_WRITE);
	if (!dstf) return ERR_UNKNOWN;

	char buffer[256];
	while (srcf->available() > 0) {
		int n_read = srcf->read(buffer, 256);
		dstf->write(buffer, n_read);
	}

	srcf->close();
	dstf->close();

	return 0;
}

bool fsexists(const char* dir) {
	for (int i = 0; i < num_mounted; i++) {
		if (str_startswith(dir, (const char*)mount_names[i])) {
			dir += strlen((const char*)mount_names[i]);
			FileSystemDevice* fs = mount_fs[i];
			return fs->exists(dir);
		}
	}
	return false;
}
