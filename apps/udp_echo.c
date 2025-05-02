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
        udp_sendto(pcb, p, addr, port);
        pbuf_free(p);
    }
}

/**
 * @brief Initialize UDP connect
 *        Local port: 2300, Remote port: 1000
 */
void udpecho_init(void)
{
    udp_echo_pcb = udp_new();

    udp_bind(udp_echo_pcb, IP_ADDR_ANY, 2300);
    udp_connect(udp_echo_pcb, IP_ADDR_ANY, 1000);
    udp_recv(udp_echo_pcb, udpecho_recv, NULL);
}
