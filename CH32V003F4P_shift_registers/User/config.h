/********************************** (C) COPYRIGHT *******************************
 * File Name          : config
 * Author             : Tauno Erik
 * Version            : V1.0.0
 * Date               : 2026-11-03
 * Description        : Seaded
 *******************************************************************************/

// USART1
#define APP_BAUDRATE   115200

// Hardware SPI 1 pins - connected to Shift Registers
#define SPI1_PORT  GPIOC
#define DATA_PIN  GPIO_Pin_6 // MOSI
#define CLOCK_PIN GPIO_Pin_5 // SCK
#define LATCH_PIN GPIO_Pin_1



#define PATERN 0b111