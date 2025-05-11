#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <string.h>

#define GPS_MSG_SIZE 80

int init_gps_service();

void serial_cb(const struct device *dev, void *user_data);

int get_date_time(char* datetime);

int get_sentence(char *sentence_type, char *sentence);
