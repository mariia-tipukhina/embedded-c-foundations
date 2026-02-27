#include <stdint.h>

void delay(volatile uint32_t count) {
	while (count--) {

	}
}

volatile uint32_t* pClkCtrlReg = (uint32_t*)0x40021018; //RCC_APB2ENR
volatile uint32_t* pPortAModeReg = (uint32_t*)0x40010804; //GPIOx_A(CRH)
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
	*pClkCtrlReg |= (1 << 2) | (1 << 14);

	//pin muxing
	*pPortAModeReg &= ~(0xF << 4);
	*pPortAModeReg |= (0xB << 4);

	//peripheral config
	*pUsart1BaudReg = 0x341;
	*pUsart1CtrlReg |= (1 << 13); //UE
	*pUsart1CtrlReg |= (1 << 3); //TE

	while (1)
	{

		UART_SendString("Mariia Tipukhina is sending a message\r\n");
		delay(1000000);

	}
}