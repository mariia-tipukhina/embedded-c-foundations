#include <stdint.h>

void delay(volatile uint32_t count) {
    while (count--) {

    }
}

int main(void)
{

    volatile uint32_t* pClkCtrlReg = (uint32_t*)0x40021018;
    volatile uint32_t* pPortBModeReg = (uint32_t*)0x40010C00;
    volatile uint32_t* pPortBOutReg = (uint32_t*)0x40010C0C;

    *pClkCtrlReg |= (1 << 3);

    *pPortBModeReg &= ~(0xF << 8);
    *pPortBModeReg |= (1 << 8);



    while (1)
    {
        //hearbeat simulation 
        *pPortBOutReg &= ~(1 << 2);
        delay(100000);
        *pPortBOutReg |= (1 << 2);

        delay(150000);

        *pPortBOutReg &= ~(1 << 2);
        delay(100000);
        *pPortBOutReg |= (1 << 2);

        delay(800000);
    }
}
