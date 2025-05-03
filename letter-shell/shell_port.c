#include "shell.h"
#include <stm32f4xx_hal.h>
#include "usart.h"
#include "shell_port.h"

/* 1. 创建shell对象，开辟shell缓冲区 */
Shell shell;
char shell_buffer[512];


/* 2. 自己实现shell写函数 */

//shell写函数原型：typedef void (*shellWrite)(const char);
short User_Shell_Write(char *data, unsigned short len)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)data, len, 0xFFFF);
    return len;
}

/* 3. 编写初始化函数 */
void User_Shell_Init(void)
{
 //注册自己实现的写函数
   shell.write = User_Shell_Write;
 
 //调用shell初始化函数
    shellInit(&shell, shell_buffer, 512);
}