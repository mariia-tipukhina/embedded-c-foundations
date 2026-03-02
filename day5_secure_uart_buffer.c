#include <stdint.h>
#include <string.h>

#define MAX_BUFFER_SIZE 16

// ==========================================
// Registers' addresses 
// ==========================================
volatile uint32_t* pClkCtrlReg = (uint32_t*)0x40021018;
volatile uint32_t* pPortAModeReg = (uint32_t*)0x40010804;
volatile uint32_t* pPortBModeReg = (uint32_t*)0x40010C00;
volatile uint32_t* pPortBOutReg = (uint32_t*)0x40010C0C;
volatile uint32_t* pUsart1CtrlReg = (uint32_t*)0x4001380C;
volatile uint32_t* pUsart1StatusReg = (uint32_t*)0x40013800;
volatile uint32_t* pUsart1DataReg = (uint32_t*)0x40013804;
volatile uint32_t* pUsart1BaudReg = (uint32_t*)0x40013808;


// ==========================================
// Drivers
// ==========================================

void UART_SendChar(char c) {
    while (!(*pUsart1StatusReg & (1 << 7))) {}
    *pUsart1DataReg = c;
}

void UART_SendString(const char* str) {
    while (*str) {
        UART_SendChar(*str++);
    }
}

void System_Init(void) {
    // Clocking
    *pClkCtrlReg |= (1 << 2) | (1 << 3) | (1 << 14);

    // Pin muxing
    *pPortAModeReg &= ~(0xF << 4);
    *pPortAModeReg |= (0xB << 4);
    *pPortAModeReg &= ~(0xF << 8);
    *pPortAModeReg |= (0x4 << 8);

    // LED pin muxing (Active-High)
    *pPortBModeReg &= ~(0xF << 8);
    *pPortBModeReg |= (1 << 8);
    *pPortBOutReg &= ~(1 << 2); 

    // UART (9600)
    *pUsart1BaudReg = 0x341;
    *pUsart1CtrlReg |= (1 << 13) | (1 << 3) | (1 << 2);
}


// ==========================================
// CLI logic
// ==========================================

// Data Hiding
static char rx_buffer[MAX_BUFFER_SIZE];
static uint8_t buffer_index = 0;

void CLI_ProcessChar(char received) {

    if (received == '\b' || received == 127) {
        if (buffer_index > 0) {
            buffer_index--;
            UART_SendString("\b \b");
        }
        return; 
    }

    if (received == '\r' || received == '\n') {
        if (buffer_index > 0) {
            rx_buffer[buffer_index] = '\0'; 

            if (strcmp(rx_buffer, "LED_ON") == 0) {
                *pPortBOutReg |= (1 << 2);
                UART_SendString("\r\nOK: LED ON\r\n");
            }
            else if (strcmp(rx_buffer, "LED_OFF") == 0) {
                *pPortBOutReg &= ~(1 << 2); 
                UART_SendString("\r\nOK: LED OFF\r\n");
            }
            else if (strcmp(rx_buffer, "HELP") == 0) {
                UART_SendString("\r\n--- MENU ---\r\nLED_ON\r\nLED_OFF\r\nHELP\r\n------------\r\n");
            }
            else {
                UART_SendString("\r\nERROR: Unknown command\r\n");
            }

            buffer_index = 0;
            UART_SendString("Ready > ");
        }
        else {
            UART_SendString("\r\nReady > ");
        }
        return; 
    }


    if (buffer_index < MAX_BUFFER_SIZE - 1) {
        UART_SendChar(received); // Echo
        rx_buffer[buffer_index] = received; 
        buffer_index++;
    }
    else {
        UART_SendString("\r\nERROR: Too long!\r\nReady > ");
        buffer_index = 0;
    }
}


int main(void) {

    System_Init();

    //CLI
    UART_SendString("\r\n==============================\r\n");
    UART_SendString("   STM32 Command Shell v1.0   \r\n");
    UART_SendString("==============================\r\n");
    UART_SendString("Ready > ");

    while (1) {

        // RXNE
        if (*pUsart1StatusReg & (1 << 5)) {

            char received = *pUsart1DataReg;

            CLI_ProcessChar(received);
        }
    }
}