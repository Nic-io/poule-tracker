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
/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_gps)

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/" DISK_DRIVE_NAME ":"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};
#define FS_RET_OK FR_OK

LOG_MODULE_REGISTER(main);
#define MSG_SIZE 320
#define MAX_LINE_LEN 100
static const char *disk_mount_pt = DISK_MOUNT_PT;

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

int main(void)
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
	struct fs_file_t file;
	int base = strlen(disk_mount_pt);

	fs_file_t_init(&file);

	LOG_INF("Creating some dir entries in %s", disk_mount_pt);
	strncpy(path, disk_mount_pt, sizeof(path));
	path[base++] = '/';
	path[base] = 0;
	strcat(&path[base], "pos.cvs");

	if (fs_open(&file, path, FS_O_CREATE | FS_O_RDWR | FS_O_TRUNC ) != 0) {
		LOG_ERR("Failed to create file %s", path);
	}

	int i=0;
    while (1) {
		char ggaframe[MAX_LINE_LEN];
		int size = get_sentence("$GPRMC", ggaframe);
		LOG_INF("RMC size %d", size);
		LOG_INF("%s", ggaframe);
		fs_write(&file,&ggaframe,size);
		fs_sync(&file);
		k_sleep(K_SECONDS(5));
		i++;
    }

	fs_close(&file);

	printk("done");
	return 0;
}

int get_sentence(char *sentence_type, char*sentence)
{
	char line[MAX_LINE_LEN];
    int idx = 0;
    bool collecting = false;

	while (1) {
		char ch;
        uart_poll_in(uart_dev, &ch);
        if (ch == EOF) {
            break;
        }
        if (ch == '$') {
            // Start of a new sentence
            collecting = true;
            idx = 0;
            line[idx++] = ch;
        } else if (collecting) {
            // Keep collecting bytes into the line buffer
            if (idx < MAX_LINE_LEN - 1) {
                line[idx++] = ch;
            }
            // End of sentence is marked by newline
            if (ch == '\n') {

                line[idx] = '\0'; // null-terminate the sentence
				//printk("%s",line);
				LOG_INF("%s", line);
                if (strstr(line, sentence_type) == line) {
                    // It’s a GGA sentence — write it to file
					strcpy(sentence,line);
					return idx;
                }
                collecting = false;
            }
        }
	k_sleep(K_USEC(1));
    }
}
