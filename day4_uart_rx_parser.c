#include <stdint.h>

volatile uint32_t* pClkCtrlReg = (uint32_t*)0x40021018; //RCC_APB2ENR
volatile uint32_t* pPortAModeReg = (uint32_t*)0x40010804; //GPIOx_A(CRH)PA9, PA10
volatile uint32_t* pPortBModeReg = (uint32_t*)0x40010C00; //GPIOx_B (CRL) PB2
volatile uint32_t* pPortBOutReg = (uint32_t*)0x40010C0C; //GPIO_ODR
volatile uint32_t* pUsart1CtrlReg = (uint32_t*)0x4001380C; //CR1
volatile uint32_t* pUsart1StatusReg = (uint32_t*)0x40013800; //SR
volatile uint32_t* pUsart1DataReg = (uint32_t*)0x40013804; //DR
volatile uint32_t* pUsart1BaudReg = (uint32_t*)0x40013808; //BRR


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
	*pPortBOutReg |= (1 << 2);

	//peripheral config
	*pUsart1BaudReg = 0x341;
	*pUsart1CtrlReg |= (1 << 13); //UE
	*pUsart1CtrlReg |= (1 << 3); //TE
	*pUsart1CtrlReg |= (1 << 2); // RE

	while (1)
	{
		//Checking RXNE (1 - there is a letter, 0 - there is not)
		if (*pUsart1StatusReg & (1 << 5)) {
			char received = *pUsart1DataReg;
			if (received == '1') {
				*pPortBOutReg |= (1 << 2);
			}
			else if (received == '0') {
				*pPortBOutReg &= ~(1 << 2);
			}
		}
	}
}
