#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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
volatile uint32_t* pNvicIser1 = (uint32_t*)0xE000E104;
volatile uint32_t* pSysTickCtrl = (uint32_t*)0xE000E010;
volatile uint32_t* pSysTickLoad = (uint32_t*)0xE000E014;
volatile uint32_t* pSysTickVal = (uint32_t*)0xE000E018;


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
    *pUsart1CtrlReg |= (1 << 13) | (1 << 3) | (1 << 2) | (1 << 5);

    //NVIC
    *pNvicIser1 |= (1 << 5);

    //SysTick
    *pSysTickLoad = 7999;
    *pSysTickVal = 0;
    *pSysTickCtrl = 0x07;
}


// ==========================================
// CLI logic
// ==========================================

// Data Hiding
static char rx_buffer[MAX_BUFFER_SIZE];
static uint8_t buffer_index = 0;

volatile uint32_t system_ms_ticks = 0;
volatile char g_ReceivedChar = 0;
volatile uint8_t g_CharReady = 0;
uint8_t is_blinking = 0;
uint32_t blink_interval = 500;

void UART_SendInt(uint32_t num) {
    char buf[11];
    int i = 0;

    if (num == 0) {
        UART_SendChar('0');
        return;
    }

    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i > 0) {
        UART_SendChar(buf[--i]);
    }
}

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
                is_blinking = 0;
                *pPortBOutReg |= (1 << 2);
                UART_SendString("\r\nOK: LED ON\r\n");
            }
            else if (strcmp(rx_buffer, "LED_OFF") == 0) {
                is_blinking = 0;
                *pPortBOutReg &= ~(1 << 2);
                UART_SendString("\r\nOK: LED OFF\r\n");
            }
            else if (strncmp(rx_buffer, "BLINK_", 6) == 0) {
                int new_interval = atoi(&rx_buffer[6]);
                if (new_interval > 10) {
                    blink_interval = new_interval;
                    is_blinking = 1;
                    UART_SendString("\r\nOK: Interval set to ");
                    UART_SendInt(blink_interval);
                    UART_SendString("ms\r\n");
                }
                else {
                    UART_SendString("\r\nERROR: Too fast!\r\n");
                }
            }
            else if (strcmp(rx_buffer, "BLINK") == 0) {
                is_blinking = 1;
                UART_SendString("\r\nOK: LED AUTO-BLINK ENABLED\r\n");
            }
            else if (strcmp(rx_buffer, "HELP") == 0) {
                UART_SendString("\r\n--- MENU ---\r\nLED_ON\r\nLED_OFF\r\nBLINK(def)\r\nBLINK_(ms)\r\nHELP\r\n------------\r\n");
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

// =======================================
//	Interrupt
// =======================================
void USART1_IRQHandler(void) {

    if (*pUsart1StatusReg & (1 << 5)) {

        g_ReceivedChar = *pUsart1DataReg;
        g_CharReady = 1;
    }
}

// =======================================
// SysTick
// =======================================
void SysTick_Handler(void) {
    system_ms_ticks++;
}

int main(void) {

    System_Init();

    uint32_t last_blink_time = 0;

    //CLI
    UART_SendString("\r\n==============================\r\n");
    UART_SendString("   STM32 Command Shell v1.0   \r\n");
    UART_SendString("==============================\r\n");
    UART_SendString("Ready > ");

    while (1) {

        if (g_CharReady == 1) {
            CLI_ProcessChar(g_ReceivedChar);
            g_CharReady = 0;
        }

        if (is_blinking) {
            if (system_ms_ticks - last_blink_time >= blink_interval) {
                last_blink_time = system_ms_ticks;
                *pPortBOutReg ^= (1 << 2);
            }
        }
    }
}
