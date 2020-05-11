#pragma once 
#include <Chip/Unknown/STMicro/STM32F103xx/FSMC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/PWR.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/RCC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOA.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOB.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOD.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOE.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOF.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/GPIOG.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/AFIO.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/EXTI.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/DMA1.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/DMA2.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/SDIO.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/RTC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/BKP.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/IWDG.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/WWDG.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM1.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM8.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM2.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM3.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM4.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM5.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM9.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM12.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM10.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM11.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM13.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM14.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM6.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/TIM7.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/I2C1.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/I2C2.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/SPI1.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/SPI2.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/SPI3.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/USART1.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/USART2.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/USART3.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/ADC1.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/ADC2.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/ADC3.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/CAN.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/DAC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/DBG.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/UART4.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/UART5.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/CRC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/FLASH.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/NVIC.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/USB.hpp>
#include <Chip/Unknown/STMicro/STM32F103xx/Sys_Tick.hpp>

#include <Chip/Unknown/STMicro/STM32F103xx/GPIO_generic.hpp>

namespace Kvasir {
  
  //TODO: manually fill this up or find a reliable source to replace
  enum IRQ : nvic::irq_number_t {
    systick_irqn = -1,
    dma_channel5_irqn = 15,
    exti_9_5_irqn = 23,
    tim1_cc_irqn = 27,
    tim2_irqn = 28,
    tim3_irqn = 29,
    tim4_irqn = 30,
    usart1_irqn = 37
  };
  
}
