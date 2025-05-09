/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ch390.h"
#include <stdio.h>
#include <string.h>
#include "my_macro.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define xprintf(fmt, ...) \
    do { \
        snprintf((char*)uart1_tx_buf, sizeof(uart1_tx_buf), fmt, ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart1, uart1_tx_buf, strlen((char*)uart1_tx_buf), 5000); \
    } while(0)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t uart1_tx_buf[128];
// phy link status
uint8_t CH390_1_phy_linked = 0;
uint8_t CH390_2_phy_linked = 0;
uint8_t CH390_3_phy_linked = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void dwt_delay_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 使能DWT
    DWT->CYCCNT = 0;                                // 复位计数器
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 使能计数
}

/**
 * @name ch390_print_info
 * @brief Read and print chip's ID and MAC address
 */
void ch390_print_info(CH390_DEVICE_T dev)
{
  uint8_t i;
  uint8_t mac_addr[6];
  uint16_t vid = ch390_get_vendor_id(dev);
  uint16_t pid = ch390_get_product_id(dev);
  xprintf("ID: %04x%04x\r\n", vid, pid);

  ch390_get_mac(CH390_DEVICE_1, mac_addr);
  xprintf("MAC: ");
  for (i = 0; i < 6; i++)
  {
      xprintf("%02X ", mac_addr[i]);
  }
  xprintf("\r\n");
}

// Random packet data for send test
#define TEST_DATA_LEN 600
#define CH390_PKT_MAX   1536    /* Received packet max size */
uint8_t ch390_1_packet_data[TEST_DATA_LEN];
uint8_t ch390_1_receive_buff[CH390_PKT_MAX];

uint8_t ch390_2_packet_data[TEST_DATA_LEN];
uint8_t ch390_2_receive_buff[CH390_PKT_MAX];

uint8_t ch390_3_packet_data[TEST_DATA_LEN];
uint8_t ch390_3_receive_buff[CH390_PKT_MAX];

void init_packet_data()
{
  int i;
  for(i = 0; i < TEST_DATA_LEN; i++)
  {
    ch390_1_packet_data[i] = i;
    ch390_2_packet_data[i] = i;
    ch390_3_packet_data[i] = i;
  }
}

void print_packet(uint8_t *buff, uint16_t length)
{
		int i;
    xprintf("------------------\r\n"
           "Packet length: %d", length);

    for(i = 0; i < length; i++)
    {
        if(i % 16 == 0)
        {
            xprintf("\r\n%04x   ", i);
        }
        xprintf("%02x ", buff[i]);
    }

    xprintf("\r\n");
}

/**
 * @name ch390_int_handler
 * @brief Handle CH390 interrupt events, include packet receive
 */
void ch390_int_handler(CH390_DEVICE_T dev)
{
  uint8_t int_status = ch390_get_int_status(dev);
  // Link status changed
  if(int_status & ISR_LNKCHG)
  {
    HAL_Delay(65);
    CH390_1_phy_linked = ch390_get_link_status(dev);
    xprintf("Link status: %d\r\n", CH390_1_phy_linked);
    ch390_write_reg(dev, CH390_ISR, ISR_LNKCHG);
  }
  // Receive overflow
  if(int_status & ISR_ROS) 
  {
    xprintf("Receive overflow\r\n");
    uint8_t rx_status = 0;
    uint32_t length = 0;
      while((length = ch390_receive_packet(dev, 
        dev == CH390_DEVICE_1 ? ch390_1_receive_buff : 
        dev == CH390_DEVICE_2 ? ch390_2_receive_buff : 
        ch390_3_receive_buff, 
        &rx_status)) != 0)
      {
          if(dev == CH390_DEVICE_1) {
              xprintf("dev 1 Rx: %lu\r\n", length);
              print_packet(ch390_1_receive_buff, length);
          } else if(dev == CH390_DEVICE_2) {
              xprintf("dev 2 Rx: %lu\r\n", length);
              print_packet(ch390_2_receive_buff, length);
          } else if(dev == CH390_DEVICE_3) {
              xprintf("dev 3 Rx: %lu\r\n", length);
              print_packet(ch390_3_receive_buff, length);
          }
      }
  }
  // Receive overflow counter overflow
  if(int_status & ISR_ROO) xprintf("Overflow counter overflow\r\n");
  // Packet transmitted
//  if(int_status & ISR_PT) printf("Packet sent\r\n");
  // Packet received
  if(int_status & ISR_PR)
  {
      /* Multiple packets may be received in a single packet receive
       * event. So "ch390_receive_packet" should be called until the
       * return value is 0. Otherwise data will accumulate in RX SRAM
       * and cause overflow. */
      uint8_t rx_status = 0;
      uint32_t length = 0;
      while((length = ch390_receive_packet(dev, 
        dev == CH390_DEVICE_1 ? ch390_1_receive_buff : 
        dev == CH390_DEVICE_2 ? ch390_2_receive_buff : 
        ch390_3_receive_buff, 
        &rx_status)) != 0)
      {
          if(dev == CH390_DEVICE_1) {
              xprintf("dev 1 Rx: %lu\r\n", length);
              print_packet(ch390_1_receive_buff, length);
          } else if(dev == CH390_DEVICE_2) {
              xprintf("dev 2 Rx: %lu\r\n", length);
              print_packet(ch390_2_receive_buff, length);
          } else if(dev == CH390_DEVICE_3) {
              xprintf("dev 3 Rx: %lu\r\n", length);
              print_packet(ch390_3_receive_buff, length);
          }
      }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  int a = 0;
  a ++;
  a ++;
  a ++;
  a ++;
  a ++;
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  dwt_delay_init();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  ch390_software_reset(CH390_DEVICE_1);
  HAL_Delay(10);
  ch390_default_config(CH390_DEVICE_1);
  ch390_print_info(CH390_DEVICE_1);
  init_packet_data(CH390_DEVICE_1); 

  //ch390_send_packet(packet_data, TEST_DATA_LEN);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    for(CH390_DEVICE_T i = CH390_DEVICE_1; i < CH390_DEVICE_NUM; i ++)
    {
      if(ch390_get_int_pin(i))
      {
        ch390_int_handler(i);
      }
      if(CH390_PHY_LINKED(i))
      {
        ch390_send_packet(i, CH390_PACKET_DATA(i), TEST_DATA_LEN);
      }
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
