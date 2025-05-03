#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#define TIME_LENGTH 11  // "hhmmss.sss" + null terminator
#define DATE_LENGTH 7   // "ddmmyy" + null terminator
#define MSG_SIZE 200

int init_gps_service();

void serial_cb(const struct device *dev, void *user_data);

int get_date_time(const char* rmc_sentence, char* time_str, char* date_str);
