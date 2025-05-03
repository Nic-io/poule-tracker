#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/led_strip.h>
#include "filelogging.h"
#include "gps_data_service.h"

LOG_MODULE_REGISTER(main);

int main(void)
{
    init_logging("position.csv");

    init_gps_service();
/* -----------UART GPS----------------------- */

    // Read Message queue and filter for RMC or GGA sentences
    while (1) {
        int size;
        char rmcframe[MSG_SIZE];
        while(get_sentence("$GPRMC", rmcframe) != 0){
            k_sleep(K_USEC(1));
        }
        size = strlen(rmcframe);
        LOG_INF("RMC size %d", size);
        LOG_INF("%s", rmcframe);

        log_line(rmcframe);
        k_sleep(K_SECONDS(5));
    }

    printk("done");
    return 0;
}
