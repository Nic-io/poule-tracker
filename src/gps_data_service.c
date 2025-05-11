#include "gps_data_service.h"

LOG_MODULE_REGISTER(gps_data);

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_gps)

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, GPS_MSG_SIZE, 10, 4);

/* receive buffer used in UART ISR callback */
static char rx_buf[GPS_MSG_SIZE];
static int rx_buf_pos;

int init_gps_service(void)
{
    /* configure interrupt and callback to receive data */
    int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

    if (ret < 0) {
        if (ret == -ENOTSUP) {
            printk("Interrupt-driven UART API support not enabled\n");
        } else if (ret == -ENOSYS) {
            printk("UART device does not support interrupt-driven API\n");
        } else {
            printk("Error setting UART callback: %d\n", ret);
        }
        return ret;;
    }
    uart_irq_rx_enable(uart_dev);
    return 0;
}

void serial_cb(const struct device *dev, void *user_data)
{
    uint8_t c;
    if (!uart_irq_update(uart_dev)) {
        return;
    }

    if (!uart_irq_rx_ready(uart_dev)) {
        return;
    }

    //read until FIFO empty
    while (uart_fifo_read(uart_dev, &c, 1) == 1) {
        if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
            // terminate string
            rx_buf[rx_buf_pos] = '\n';
            rx_buf[rx_buf_pos+1] = '\0';

            //if queue is full, drop older messages
            if(k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT) !=0 ){
                k_msgq_purge(&uart_msgq);
            }
            // reset the buffer (it was copied to the msgq)
            rx_buf_pos = 0;
        } else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
            if(c != '\n' && c != '\r'){
                rx_buf[rx_buf_pos++] = c;
            }
        }
        // else: characters beyond buffer size are dropped
    }
}

int get_sentence(char *sentence_type, char *sentence)
{
    char buffer[GPS_MSG_SIZE];
    int ret;
    ret = k_msgq_get(&uart_msgq, buffer, K_NO_WAIT);
    if(ret != 0){
        return ret;
    }
    if (strstr(buffer, sentence_type) == buffer) {
       // It’s a Matching sentence — write it to file
       strcpy(sentence,buffer);
       return 0;
    } else {
        return -1;
    }
}

int get_date_time(char* datetime)
{
    int date_position = 9;
    int time_position = 1;

    char buffer[GPS_MSG_SIZE];
    int buffer_pos=0;
    int outputpos=0;
    while(get_sentence("$GPRMC", buffer) != 0){
        k_sleep(K_USEC(1));
    }
/*
    //Scroll for Date
    for(int i=0; i < date_position ; i++){
        while(buffer[buffer_pos]!=','){
            buffer_pos++;
        }
        buffer_pos++;
    }
    for(int i = 0; i < 4; i++){
        datetime[outputpos]=buffer[buffer_pos];
        outputpos++;
        buffer_pos++;
    }
    datetime[outputpos++]='_';
*/
    //Scroll for time
    buffer_pos=0;
    for(int i = 0; i < time_position ; i++){
        while(buffer[buffer_pos]!=','){
            buffer_pos++;
        }
        buffer_pos++;
    }
    for(int i = 0; i < 4; i++){
        datetime[outputpos]=buffer[buffer_pos];
        outputpos++;
        buffer_pos++;
    }
    datetime[outputpos]='\0';
    LOG_INF("datetime %s", datetime);
}
