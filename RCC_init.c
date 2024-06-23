#include "stm32f10x.h"
#define QUARZ_FREQ_HZ 8000000


void RCC_Init(void)
{
 RCC->CR |= RCC_CR_HSEON;
while ( (RCC->CR & RCC_CR_HSERDY) == 0 ) {}

 // AHB freq = 72 MHz, APB1 = 36 MHz, APB2 = 36 MHz
 RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV2 | RCC_CFGR_PPRE1_DIV2; 
 RCC->CR &= ~(RCC_CR_PLLON); // Отключение PLL перед настройкой тактирования, частота = 8 MHz
 RCC->CFGR |= RCC_CFGR_PLLSRC_HSE; // Выбор HSE в качестве источника тактирования

/* тут настройка предделителей и умножителей для PLL с целью получить требуемую частоту 72 МГц
============================================================================================= */
 RCC->CFGR &= ~(RCC_CFGR_PLLMULL);  // clear PLL multipyer value
 RCC->CFGR |= RCC_CFGR_PLLMULL9; // // Multiplyer = 9; 8 MHz * 9 = 72 MHz
 //RCC->CFGR &= ~(RCC_CFGR_PLLP); // /P = 2 - PLLCLK = 36 MHz
/* ========================================================================================== */
 RCC->CR |= RCC_CR_PLLON; // подача тактирования на PLL
 while ( (RCC->CR & RCC_CR_PLLRDY) == 0 ) {} // Ожидание стабилизации частоты PLL

/*
настройка регистров FLASH-памяти.
Эта операция обязательна и выполняется только 1 раз при старте программы
====================================================================================
*/
 //FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN;
/* ================================================================================= */
 RCC->CFGR &= ~RCC_CFGR_SW;
 RCC->CFGR |= RCC_CFGR_SW_PLL; // подключение PLL к SYSCLK. Выбор PLL в качестве
 // основного источника тактирования
 while ( (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL ) {} // Ожидание, пока PLL подключится к SYSCLK
}