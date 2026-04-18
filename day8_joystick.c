#include "stm32f1xx.h"

extern "C" {

	void EXTI1_IRQHandler(void) {
		GPIOC->BSRR = (1 << 29);
		EXTI->PR = (1 << 1);
	}

	void EXTI2_IRQHandler(void) {
		GPIOC->BSRR = (1 << 13);
		EXTI->PR = (1 << 2);
	}

}

int main(void)
{
	RCC->APB2ENR |= (1 << 4) | (1 << 2) | (1 << 0);
	GPIOC->CRH &= ~(0xF << 20);
	GPIOC->CRH |= (0x2 << 20);
	GPIOA->CRL &= ~((0xF << 4) | (0xF << 8));
	GPIOA->CRL |= (0x8 << 4) | (0x8 << 8);
	GPIOC->ODR |= (1 << 13);
	GPIOA->ODR |= (1 << 1) | (1 << 2);
	AFIO->EXTICR[0] &= ~((0xF << 4) | (0xF << 8));
	EXTI->IMR |= (1 << 1) | (1 << 2);
	EXTI->FTSR |= (1 << 1) | (1 << 2);

	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);

	while (1)
	{

	}

}


