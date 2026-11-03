/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : Tauno Erik
 * Version            : V1.0.0
 * Date               : 2026-11-03
 * Description        : Main program body.
 *******************************************************************************/

/**********************************************************************
@Note
USART1_Tx(PD5)
USART1_Rx(PD6)
 
Baud rate 115200
 
Hardware connection:
PD5 --> Rx
PD6 --> Tx
**************************************************************************/

#include "debug.h"
#include "config.h"
#include "data.h"

// Global tick counter
volatile uint32_t global_ms = 0;


/*********************************************************************
 * USART1 config
 * Baud Rate
 * PORTD
 * PD5
 * PD6
 */
void USARTx_CFG(void) {
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = APP_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

/*
void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Enable Clock for Port C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // Configure pins as Push-Pull Output, 10MHz
    GPIO_InitStructure.GPIO_Pin = DATA_PIN | CLOCK_PIN | LATCH_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIO_PORT, &GPIO_InitStructure);
}
*/

/*
// Sends 8 bits to the shift register
void ShiftOut(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        // Set Data Pin high or low based on the MSB (Most Significant Bit)
        if (data & (0x80 >> i)) {
            GPIO_WriteBit(GPIO_PORT, DATA_PIN, Bit_SET);
        } else {
            GPIO_WriteBit(GPIO_PORT, DATA_PIN, Bit_RESET);
        }

        // Pulse the Shift Clock (SH_CP)
        GPIO_WriteBit(GPIO_PORT, CLOCK_PIN, Bit_SET);
        GPIO_WriteBit(GPIO_PORT, CLOCK_PIN, Bit_RESET);
    }

    // Pulse the Latch Clock (ST_CP) to update the outputs
    GPIO_WriteBit(GPIO_PORT, LATCH_PIN, Bit_SET);
    GPIO_WriteBit(GPIO_PORT, LATCH_PIN, Bit_RESET);
}
*/

// Update the global tick every 1ms
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void) {
    global_ms++;
    SysTick->SR = 0; // Clear interrupt flag
}

void SysTick_Init(void) {
    // SystemCoreClock is usually 48MHz
    // Set reload to trigger every 1ms
    SysTick->CMP = SystemCoreClock / 1000;
    SysTick->CNT = 0;
    SysTick->CTLR = 0x0F; // Enable interrupt, enable counter, HCLK as clock
    NVIC_EnableIRQ(SysTick_IRQn);
}

/*
 *
 */
void SPI_1_config(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1, ENABLE);

    // PC5 (SCK) & PC6 (MOSI) as Alternate Function Push-Pull
    GPIO_InitStructure.GPIO_Pin = CLOCK_PIN | DATA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI1_PORT, &GPIO_InitStructure);

    // PC1 (Latch) as standard Output
    GPIO_InitStructure.GPIO_Pin = LATCH_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI1_PORT, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 3MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}


/*
 * Sends 4 bytes (32 bits)
 */
void SPI_send_data(uint32_t data) {
    GPIO_WriteBit(SPI1_PORT, LATCH_PIN, Bit_RESET); // Latch LOW

    for(int i = 3; i >= 0; i--) {
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData(SPI1, (uint8_t)(data >> (i * 8)));
    }

    // Wait for last byte to finish
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

    GPIO_WriteBit(SPI1_PORT, LATCH_PIN, Bit_SET); // Latch HIGH
}


// 32-bitine paremale p??ramine
uint32_t rotate_right_32(uint32_t value, uint8_t shift) {
    shift &= 31; // Tagab, et nihe on vahemikus 0-31
    if (shift == 0) return value;
    return (value >> shift) | (value << (32 - shift));
}

// 32-bitine vasakule p??ramine
uint32_t rotate_left_32(uint32_t value, uint8_t shift) {
    shift &= 31;
    if (shift == 0) return value;
    return (value << shift) | (value >> (32 - shift));
}


int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    //GPIO_Config();
    SPI_1_config();
    SysTick_Init();

    USARTx_CFG();

#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(APP_BAUDRATE);
#endif

    printf("SystemClk:%d\r\n",SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());

    

    //uint8_t pattern = 0x01;
    uint32_t data = PATERN;
    uint32_t last_update_time = 0;
    const uint32_t interval = 90; // ms update rate

    SPI_send_data(0);

    while(1)
    {
        if (global_ms - last_update_time >= interval) {
            last_update_time = global_ms;

            SPI_send_data(data);
            
            //data = rotate_left_32(data, 1);
            data = rotate_right_32(data, 1);
            
        }
    } // while end
}

