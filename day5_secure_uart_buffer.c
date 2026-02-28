#include <stdint.h>
#include <string.h>
#define MAX_BUFFER_SIZE 16

volatile uint32_t* pClkCtrlReg = (uint32_t*)0x40021018; //RCC_APB2ENR
volatile uint32_t* pPortAModeReg = (uint32_t*)0x40010804; //GPIOx_A(CRH)PA9, PA10
volatile uint32_t* pPortBModeReg = (uint32_t*)0x40010C00; //GPIOx_B (CRL) PB2
volatile uint32_t* pPortBOutReg = (uint32_t*)0x40010C0C; //GPIO_ODR
volatile uint32_t* pUsart1CtrlReg = (uint32_t*)0x4001380C; //CR1
volatile uint32_t* pUsart1StatusReg = (uint32_t*)0x40013800; //SR
volatile uint32_t* pUsart1DataReg = (uint32_t*)0x40013804; //DR
volatile uint32_t* pUsart1BaudReg = (uint32_t*)0x40013808; //BRR

void UART_SendChar(char c) {
    while (!(*pUsart1StatusReg & (1 << 7))) {}
    *pUsart1DataReg = c;
}

void UART_SendString(const char* str) {
    while (*str) {
        UART_SendChar(*str++);
    }
}

int main(void)
{

    //clocking
    *pClkCtrlReg |= (1 << 2) | (1 << 3) | (1 << 14);

    //pin muxing
    *pPortAModeReg &= ~(0xF << 4); //PA9
    *pPortAModeReg |= (0xB << 4);
    *pPortAModeReg &= ~(0xF << 8); //PA10
    *pPortAModeReg |= (0x4 << 8);

    *pPortBModeReg &= ~(0xF << 8); //PB2
    *pPortBModeReg |= (1 << 8);
    *pPortBOutReg &= ~(1 << 2); //LED off

    //peripheral config
    *pUsart1BaudReg = 0x341;
    *pUsart1CtrlReg |= (1 << 13); //UE
    *pUsart1CtrlReg |= (1 << 3); //TE
    *pUsart1CtrlReg |= (1 << 2); // RE

    char rx_buffer[MAX_BUFFER_SIZE];
    uint8_t buffer_index = 0;

    UART_SendString("\r\n==============================\r\n");
    UART_SendString("   STM32 Command Shell v1.0   \r\n");
    UART_SendString("==============================\r\n");
    UART_SendString("Type 'HELP' to see all commands\r\n");
    UART_SendString("Ready > ");

    while (1)
    {

        if (*pUsart1StatusReg & (1 << 5)) { //Checking RXNE (1 - there is a letter, 0 - there is not)
            char received = *pUsart1DataReg;
            UART_SendChar(received);
            if (received == '\r' || received == '\n') {
                if (buffer_index > 0) {
                    rx_buffer[buffer_index] = '\0';
                    if (strcmp(rx_buffer, "LED_ON") == 0) {
                        *pPortBOutReg |= (1 << 2);
                        UART_SendString("OK: LED turned ON\r\n");
                    }
                    else if (strcmp(rx_buffer, "LED_OFF") == 0) {
                        *pPortBOutReg &= ~(1 << 2);
                        UART_SendString("OK: LED turned OFF\r\n");
                    }
                    else if (strcmp(rx_buffer, "HELP") == 0) {
                        UART_SendString("\r\n--- STM32 COMMAND LIST ---\r\n");
                        UART_SendString("LED_ON  - Turn the LED on\r\n");
                        UART_SendString("LED_OFF - Turn the LED off\r\n");
                        UART_SendString("HELP    - Show this message\r\n");
                        UART_SendString("--------------------------\r\n");
                    }
                    else {
                        UART_SendString("ERROR: Unknown command\r\n");
                    }
                    buffer_index = 0;
                    UART_SendString("\r\nReady > ");
                }
                else {
                    UART_SendString("\r\nReady > ");
                }
            }
            else {
                if (buffer_index < MAX_BUFFER_SIZE - 1) {
                    rx_buffer[buffer_index] = received;
                    buffer_index++;
                }
                else {
                    UART_SendString("\r\nERROR: Command too long! Buffer cleared.\r\n");
                    buffer_index = 0;
                    UART_SendString("\r\nReady > ");
                }
            }
        }
    }
}
