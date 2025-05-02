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
    IP4_ADDR(&server_ip, 192, 168, 1, 100);

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
