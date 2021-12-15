#include "../../buckler-ext/software/libraries/ydlidar_x2/ydlidar_x2.h"
#include "lidar_classification.h"
#include "sort.h"

#include <stdio.h>

#include "nrf_drv_clock.h"
#include "nrf_uarte.h"
#include "nrf_serial.h"
#include "app_timer.h"
#include "nrf_delay.h"

#include "buckler.h"

uint8_t buffer[90];
uint8_t counter = 0;

float sorted_theta[MAX_RESOLUTION];
float sorted_distance[MAX_RESOLUTION];

YdLidarData_t lidar_data;
coordinate position;

NRF_SERIAL_DRV_UART_CONFIG_DEF(m_uart0_drv_config,
                               BUCKLER_UART_RX, BUCKLER_UART_TX,
                               0, 0,
                               NRF_UART_HWFC_DISABLED, NRF_UART_PARITY_EXCLUDED,
                               NRF_UART_BAUDRATE_115200,
                               UART_DEFAULT_CONFIG_IRQ_PRIORITY);

#define SERIAL_FIFO_TX_SIZE 0
#define SERIAL_FIFO_RX_SIZE 90

NRF_SERIAL_QUEUES_DEF(serial_queue, SERIAL_FIFO_TX_SIZE, SERIAL_FIFO_RX_SIZE);

#define SERIAL_BUFF_TX_SIZE 0
#define SERIAL_BUFF_RX_SIZE 90

NRF_SERIAL_BUFFERS_DEF(serial_buffs, SERIAL_BUFF_TX_SIZE, SERIAL_BUFF_RX_SIZE);

static void ser_event_handler(nrf_serial_t const *p_serial, nrf_serial_event_t event);

NRF_SERIAL_CONFIG_DEF(serial_config, NRF_SERIAL_MODE_DMA,
                      &serial_queue, &serial_buffs, ser_event_handler, NULL);

NRF_SERIAL_UART_DEF(serial_uart, 0);

static void ser_event_handler(nrf_serial_t const *p_serial, nrf_serial_event_t event)
{
    switch (event)
    {
        case NRF_SERIAL_EVENT_TX_DONE:
        {
            break;
        }
        case NRF_SERIAL_EVENT_RX_DATA:
        {
            counter++;
            size_t read;
            nrf_serial_read(&serial_uart, buffer, sizeof(buffer), &read, 0);
            if (buffer[2] == 0) {
                ser_rx_data(read);
            }
            break;
        }
        case NRF_SERIAL_EVENT_DRV_ERR:
        {
            nrf_serial_rx_drain(&serial_uart);
            nrf_serial_uninit(&serial_uart);
            nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);
            break;
        }
        case NRF_SERIAL_EVENT_FIFO_ERR:
        {
            break;
        }
    }
}

void ser_rx_data(size_t size) {
    // Do something useful with recieved data
    //for (int i = 0; i < size; i++) {
    //    printf("%x", buffer[i]);
    //}
    __disable_irq();
    get_lidar_data(buffer, &lidar_data);    
    __enable_irq();
}

/*
    SLAVE METHOD HERE TO GIVE POSITION.
*/


int main(void) {
    nrf_serial_init(&serial_uart, &m_uart0_drv_config, &serial_config);

    while (1) {
        nrf_delay_ms(1000);
        __disable_irq();
        memcpy(sorted_theta, lidar_data.theta, sizeof(float) * MAX_RESOLUTION);
        memcpy(sorted_distance, lidar_data.distance, sizeof(float) * MAX_RESOLUTION);
        quickSort(sorted_theta, sorted_distance, 0, MAX_RESOLUTION - 1);
        classify(sorted_distance, sorted_theta, 4, &position);
        __enable_irq();
    }
}
