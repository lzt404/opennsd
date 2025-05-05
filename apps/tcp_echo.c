/********************************** (C) COPYRIGHT *****************************
 * File Name          : tcp_echo.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/08/20
 * Description        : TCP echo example.
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/

#include "tcp_echo.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "ch390.h"
#include "gpio.h"
#include "main.h"
#include "shell_port.h"

#if LWIP_TCP && LWIP_CALLBACK_API

/*********************** TCP Server ***********************/
static struct tcp_pcb *tcp_server_pcb;

/**
 * @brief Send back the received data
 */
static err_t
tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        err_t err;
        tcp_recved(tpcb, p->tot_len);

        do{
            err = tcp_write(tpcb, p->payload, p->tot_len, 1);
        }while(err != ERR_OK);

        // 打印收到的数据内容（十六进制）
        xprintf("TCP recv, len=%d: ", p->tot_len);
        struct pbuf *q = p;
        if(q->len == 3)
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
        
        pbuf_free(p);
        return ERR_OK;
    }
    else
    {
        return ERR_BUF;
    }
}

/**
 * @brief Accept connection
 */
static err_t
tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

/**
 * @brief Initialize TCP server
 *        Local port: 2300
 */
void tcp_server_init(void)
{
    tcp_server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_bind(tcp_server_pcb, IP_ANY_TYPE, 2300);
    tcp_server_pcb = tcp_listen(tcp_server_pcb);
    tcp_accept(tcp_server_pcb, tcp_server_accept);
}

/*********************** TCP Client ***********************/
static struct tcp_pcb *tcp_client_pcb;
static void tcp_client_reconnect(void *arg)
{
    xprintf("Reconnect...\n");
    tcp_client_init();
}

/**
 * @brief Send back the received data
 */
static err_t
tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        tcp_recved(tpcb, p->tot_len);
        tcp_write(tpcb, p->payload, p->tot_len, 1);
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        tcp_close(tpcb);
        xprintf("Disconnected\r\n");
        // Reconnect after 1 second
        sys_timeout(1000, tcp_client_reconnect, NULL);
    }
    return ERR_OK;
}

/**
 * @brief TCP error callback
 */
static void tcp_client_error(void *arg, err_t err)
{
    xprintf("Connection abort\n");
    // Reconnect after 1 second
    sys_timeout(1000, tcp_client_reconnect, NULL);
}

/**
 * @brief TCP connect callback
 */
static err_t
tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    xprintf("Connected\r\n");
    tcp_recv(pcb, tcp_client_recv);
    // 主动发送一条消息
    const char *msg = "hello tcp";
    tcp_write(pcb, msg, strlen(msg), 1);

    return ERR_OK;
}

/**
 * @brief Initialize TCP client
 *        Server IP: 192.168.1.100, Port: 2200
 */
void tcp_client_init(void)
{
    // extern struct netif ch390_netif;
    ip4_addr_t server_ip;
    uint16_t server_port = 2200;
    IP4_ADDR(&server_ip, 192, 168, 31, 149);

    tcp_client_pcb = tcp_new();
    if (tcp_client_pcb == NULL)
    {
        xprintf("tcp_new error\n");
        return;
    }

    tcp_bind(tcp_client_pcb, IP_ANY_TYPE, 0);
    tcp_err(tcp_client_pcb, tcp_client_error);
    tcp_connect(tcp_client_pcb, (const ip_addr_t *)&server_ip,
                server_port, tcp_client_connected);
}

#endif /* LWIP_TCP && LWIP_CALLBACK_API */
