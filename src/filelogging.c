#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <ff.h>
#include <zephyr/drivers/led_strip.h>
#include <filelogging.h>

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/" DISK_DRIVE_NAME ":"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};
#define FS_RET_OK FR_OK

LOG_MODULE_REGISTER(fslog);

static const char *disk_mount_pt = DISK_MOUNT_PT;
struct fs_file_t file;

int init_logging_tosd(char* filename)
{
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	uint64_t memory_size_mb;
	uint32_t block_count;
	uint32_t block_size;

	if (disk_access_ioctl(disk_pdrv,
						  DISK_IOCTL_CTRL_INIT, NULL) != 0) {
		printk("Storage init ERROR!");
	}

	if (disk_access_ioctl(disk_pdrv,
						  DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
		printk("Unable to get sector count");
	}
	printk("Block count %u", block_count);

	if (disk_access_ioctl(disk_pdrv,
						  DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
		printk("Unable to get sector size");
	}
	LOG_INF("Sector size %u\n", block_size);

	memory_size_mb = (uint64_t)block_count * block_size;
	LOG_INF("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

	if (disk_access_ioctl(disk_pdrv,
						  DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
		LOG_ERR("Storage deinit ERROR!");
	}

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);
	LOG_INF("mount");
	if (res == FS_RET_OK) {
		LOG_INF("mountsucces\n");
	}

	char path[128];

	int base = strlen(disk_mount_pt);

	fs_file_t_init(&file);

	LOG_INF("Creating some dir entries in %s", disk_mount_pt);
	strncpy(path, disk_mount_pt, sizeof(path));
	path[base++] = '/';
	path[base] = 0;
	strcat(&path[base], filename);

	if (fs_open(&file, path, FS_O_CREATE | FS_O_RDWR | FS_O_TRUNC ) != 0) {
		LOG_ERR("Failed to create file %s", path);
	}
}

int log_line_tosd(char *data)
{
    int size;
    size = strlen(data);
    LOG_INF("log %s , size %d", data, size);
    fs_write(&file,data,size);
    fs_sync(&file);
}

//fs_close(&file);
