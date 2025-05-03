#include "shell.h"
#include "shell_port.h"
#include <stdio.h>
#include "ch390.h"

int print_args_func(int argc, char *argv[])
{
    xprintf("%d  parameter(s)\r\n", argc);
    for (int i = 1; i < argc; i++)
    {
        xprintf("%s\r\n", argv[i]);
    }
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
