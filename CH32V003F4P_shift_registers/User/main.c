/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/25
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *Multiprocessor communication mode routine:
 *Master:USART1_Tx(PD5)\USART1_Rx(PD6).
 *This routine demonstrates that USART1 receives the data sent by CH341 and inverts
 *it and sends it (baud rate 115200).
 *
 *Hardware connection:PD5 -- Rx
 *                     PD6 -- Tx
 *
 */

#include "debug.h"


/* Global define */
// Define our pins on Port C
#define DATA_PIN  GPIO_Pin_0
#define CLOCK_PIN GPIO_Pin_1
#define LATCH_PIN GPIO_Pin_2
#define GPIO_PORT GPIOC


/* Global Variable */
vu8 val;

// Global tick counter
volatile uint32_t global_ms = 0;

/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART2 & USART3 peripheral.
 *
 * @return  none
 */
void USARTx_CFG(void)
{
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

    USART_InitStructure.USART_BaudRate = 115200;
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

void SPI_1_config(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1, ENABLE);

    // PC5 (SCK) & PC6 (MOSI) as Alternate Function Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PC1 (Latch) as standard Output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

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

void SPI_send_data(uint32_t data) {
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET); // Latch LOW

    // Send 4 bytes (32 bits)
    for(int i = 3; i >= 0; i--) {
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData(SPI1, (uint8_t)(data >> (i * 8)));
    }

    // Wait for last byte to finish
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_SET); // Latch HIGH
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); // 2?
    SystemCoreClockUpdate();
    Delay_Init();
    //GPIO_Config();
    SPI_1_config();
    SysTick_Init();

#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    printf("SystemClk:%d\r\n",SystemCoreClock);
    printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );

    USARTx_CFG();

    //uint8_t pattern = 0x01;
    uint32_t data = 0x01;
    uint32_t last_update_time = 0;
    const uint32_t interval = 15; // 100ms update rate

    while(1)
    {
        if (global_ms - last_update_time >= interval) {
            last_update_time = global_ms;

            SPI_send_data(data);

            // Rotate bits for a "running light" effect
            data = (data << 1);
            if (data == 0) {
                data = 0x01;
            }
        }
    } // while end
}
