#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/led_strip.h>
#include "filelogging.h"
#include "gps_data_service.h"

LOG_MODULE_REGISTER(main);
char filename[30];

int main(void)
{
    char *suffix = "pos.csv";

    init_gps_service();
    LOG_INF("gps ready");

    char datetime[10];
    get_date_time(datetime);

    strcat(filename, datetime);
    strcat(filename, suffix);
    LOG_INF("Opening file : %s",filename);
    init_logging_tosd(filename);
    LOG_INF("logging ready");
/* -----------UART GPS----------------------- */

    // Read Message queue and filter for RMC or GGA sentences
    int size;
    char rmcframe[GPS_MSG_SIZE];
    char *rmcheader = "$GPRMC";
    //  k_sleep(K_MSEC(1000));
    while (1) {
        while(get_sentence("$GPRMC", rmcframe) != 0){
            k_sleep(K_USEC(1));
        }
        LOG_INF("%s", rmcframe);
        size = strlen(rmcframe);

        log_line_tosd(rmcframe);
        k_sleep(K_SECONDS(5));
    }

    printk("done");
    return 0;
}
