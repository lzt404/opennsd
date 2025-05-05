#include "shell.h"
#include "shell_port.h"
#include <stdio.h>
#include "ch390.h"
#include "netif/ethernetif.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include <string.h>
#include "lwip/apps/ping.h"

int print_args_func(int argc, char *argv[])
{
    xprintf("%d  parameter(s)\r\n", argc);
    for (int i = 0; i < argc; i++)
    {
        xprintf("%s\r\n", argv[i]);
    }
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), print_args, print_args_func, test);

// 复位芯片命令函数
int reset_chip(int argc, char *argv[])
{
    xprintf("System will reset...\r\n");
    NVIC_SystemReset();
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), reset, reset_chip, reset chip);

int update_dhcp(int argc, char* argv[])
{
  ch390_print_info(CH390_DEVICE_1);
  dhcp_start(&ch390_netif);
  return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), udhcp, update_dhcp, update dhcp);

int ping_cmd(int argc, char* argv[])
{
    if (argc < 2) {
        xprintf("Usage: ping <ip>\r\n");
        return -1;
    }
    xprintf("is going to ping %s\r\n", argv[1]);
    ip_addr_t target_addr;
    if (!ipaddr_aton(argv[1], &target_addr)) {
        xprintf("Invalid IP address: %s\r\n", argv[1]);
        return -2;
    }

    xprintf("Pinging %lu ...\r\n", target_addr.addr);
    ping_init(&target_addr);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ping, ping_cmd, ping <ip>);
