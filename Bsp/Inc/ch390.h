#ifndef __CH390_H
#define __CH390_H

#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "ch390_reg.h"

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
uint32_t ch390_receive_packet(CH390_DEVICE_T dev, uint8_t *buff, uint8_t *rx_status);

int ch390_get_link_status(CH390_DEVICE_T dev);

#endif // !__CH390_H
