/********************************** (C) COPYRIGHT *****************************
 * File Name          : udp_echo.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/08/20
 * Description        : UDP echo example.
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/

#include <stddef.h>
#include <string.h>
#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"
#include "lwip/udp.h"
#include "gpio.h"
#include "main.h"
#include "ch390.h"

static struct udp_pcb *udp_echo_pcb;

/**
 * @brief Send back the received data
 *
 * @notice If received pbuf total length is larger than 1472 and send back
 *         all the data at once, they will be fragmented by IP layer, and
 *         PC may not reassemble them correctly. So it is recommended to
 *         set remote MSS to 1472
 */
static void
udpecho_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                 const ip_addr_t *addr, u16_t port)
{
    if(p != NULL)
    {
        // 打印收到的数据内容（十六进制）
        xprintf("UDP recv, len=%d: ", p->tot_len);
        struct pbuf *q = p;
        if (q->len == 3)
        {
            uint8_t *payload = (uint8_t *)q->payload;
            if (payload[0] == 's' && payload[2] == 'e')
            {
                payload[1] -= '0';
                HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, payload[1]);
            }
        }
        while (q) {
            for (u16_t i = 0; i < q->len; i++) {
                xprintf("%02x ", ((uint8_t*)q->payload)[i]);
            }
            q = q->next;
        }
        xprintf("\r\n");

        // 回显数据（如需）
        // udp_sendto(pcb, p, addr, port);
        pbuf_free(p);
    }
}

/**
 * @brief Initialize UDP connect
 *        Local port: 8000, Remote port: 1000
 */
void udpecho_init(void)
{
    udp_echo_pcb = udp_new();
    udp_bind(udp_echo_pcb, IP_ADDR_ANY, 8000);
    //udp_connect(udp_echo_pcb, IP_ADDR_ANY, 1000);
    udp_recv(udp_echo_pcb, udpecho_recv, NULL);
}
