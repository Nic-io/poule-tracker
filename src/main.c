#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/led_strip.h>
//#include "filelogging.h"
#include "gps_data_service.h"

LOG_MODULE_REGISTER(main);

int main(void)
{
    init_logging_tosd("psi.csv");
    LOG_INF("logging ready");

    init_gps_service();
    LOG_INF("gps ready");

    char time[10];
    char date[10];
    get_date_time(date ,time);

/* -----------UART GPS----------------------- */

    // Read Message queue and filter for RMC or GGA sentences
    int size;
    volatile char test[300];
    char rmcframe[GPS_MSG_SIZE];
    char *rmcheader = "$GPRMC";
    int ret =-1;
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
