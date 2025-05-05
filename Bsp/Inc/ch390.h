#ifndef __CH390_H
#define __CH390_H

#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "string.h"
#include "spi.h"
#include "ch390_reg.h"

extern uint8_t uart1_tx_buf[1600];

#define xprintf(fmt, ...) \
    do { \
        snprintf((char*)uart1_tx_buf, sizeof(uart1_tx_buf), fmt, ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, uart1_tx_buf, strlen((char*)uart1_tx_buf), 5000); \
    } while(0)

// Packet status
#define CH390_PKT_NONE  0x00    /* No packet received */
#define CH390_PKT_RDY   0x01    /* Packet ready to receive */
#define CH390_PKT_ERR   0xFE    /* Un-stable states */
#define CH390_PKT_MAX   1536    /* Received packet max size */
#define CH390_PKT_MIN   64

#define CH390_PHY_LINKED(i)   ((i) == CH390_DEVICE_1 ? CH390_1_phy_linked : \
                               (i) == CH390_DEVICE_2 ? CH390_2_phy_linked : \
                               CH390_3_phy_linked)

#define CH390_PACKET_DATA(i)  ((i) == CH390_DEVICE_1 ? ch390_1_packet_data : \
                               (i) == CH390_DEVICE_2 ? ch390_2_packet_data : \
                               ch390_3_packet_data)

/* ch390_device definition */ 
typedef enum 
{
    CH390_DEVICE_1,   
    CH390_DEVICE_2,  
    CH390_DEVICE_3,

    CH390_DEVICE_NUM,
}CH390_DEVICE_T;

#define ch390_cs(dev, value)  \
    do { \
        if ((dev) == CH390_DEVICE_1) HAL_GPIO_WritePin(CH390_CS1_GPIO_Port, CH390_CS1_Pin, (value) ? GPIO_PIN_SET : GPIO_PIN_RESET); \
        else if ((dev) == CH390_DEVICE_2) HAL_GPIO_WritePin(CH390_CS2_GPIO_Port, CH390_CS2_Pin, (value) ? GPIO_PIN_SET : GPIO_PIN_RESET); \
        else if ((dev) == CH390_DEVICE_3) HAL_GPIO_WritePin(CH390_CS3_GPIO_Port, CH390_CS3_Pin, (value) ? GPIO_PIN_SET : GPIO_PIN_RESET); \
    } while(0)

void ch390_software_reset(CH390_DEVICE_T dev);
void ch390_get_mac(CH390_DEVICE_T dev, uint8_t *mac_addr);
uint16_t ch390_get_product_id(CH390_DEVICE_T dev);
uint16_t ch390_get_vendor_id(CH390_DEVICE_T dev);
void ch390_read_mem(CH390_DEVICE_T dev, uint8_t *data, int length);
void ch390_send_request(CH390_DEVICE_T dev);
void ch390_write_mem(CH390_DEVICE_T dev, uint8_t *data, int length);
enum ch390_phy_mode
{
    CH390_10MFD,   // 10M full-duplex
    CH390_100MFD,  // 100M full-duplex
    CH390_AUTO,    // Auto negotiation
};

#define ch390_get_int_pin(dev) ( \
    (dev) == CH390_DEVICE_1 ? HAL_GPIO_ReadPin(CH390_CS1_GPIO_Port, CH390_CS1_Pin) : \
    (dev) == CH390_DEVICE_2 ? HAL_GPIO_ReadPin(CH390_CS2_GPIO_Port, CH390_CS2_Pin) : \
    (dev) == CH390_DEVICE_3 ? HAL_GPIO_ReadPin(CH390_CS3_GPIO_Port, CH390_CS3_Pin) : \
    0 \
)

uint8_t ch390_get_int_status(CH390_DEVICE_T dev);

uint8_t ch390_read_reg(CH390_DEVICE_T dev, uint8_t reg);
void ch390_write_reg(CH390_DEVICE_T dev, uint8_t reg, uint8_t value);

void ch390_default_config(CH390_DEVICE_T dev);

void ch390_send_packet(CH390_DEVICE_T dev, uint8_t *buff, uint16_t length);
void ch390_drop_packet(uint16_t len);

uint32_t ch390_receive_packet(CH390_DEVICE_T dev, uint8_t *buff, uint8_t *rx_status);
void ch390_print_info(CH390_DEVICE_T dev);
void ch390_int_handler(CH390_DEVICE_T dev);
int ch390_get_link_status(CH390_DEVICE_T dev);

#endif // !__CH390_H
