#include "mcu.hpp"
//#include <Chip/STM32F103xx.hpp>
//#include <Register/Register.hpp>
//#include <Register/Utility.hpp>

extern "C"
void SystemInit() {
  using namespace Kvasir;
  // Clock setup
  // Switch to HSE and Use pll clock
  apply(set(RccCr::hseon));
  for (auto hse_ready = read(RccCr::hserdy); !apply(hse_ready); );
  apply(
    write(RccCfgr::hpre, 0), // div1
    write(RccCfgr::ppre2, 0), // apb2 : div1
    write(RccCfgr::ppre1, 4), // apb1 : div2
    set(RccCfgr::pllsrc), // source HSE
    write(RccCfgr::pllmul, 0x7) //pllmul:9
  );
  apply(set(RccCr::pllon)); // enable pll
  for(auto pllrdy = read(RccCr::pllrdy); !apply(pllrdy); );
  
  apply(write(FlashAcr::latency, 2u)); // Flash wait state 2
  
  apply(write(RccCfgr::sw, RccCfgr::sw_val::pll)); // Clock switch : source PLL
  for (auto swrdy = read(RccCfgr::sws); apply(swrdy) != RccCfgr::sw_val::pll; );
  
  // Peripheral setup
  // enable clocks
  apply(
    set(RccApb2enr::iopcen),
    set(RccApb2enr::iopben), 
    set(RccApb2enr::iopaen),
    set(RccApb2enr::afioen),
    set(RccApb1enr::tim4en),
    set(RccApb1enr::tim3en),
    set(RccApb1enr::tim2en),
    set(RccApb2enr::tim1en),
    set(RccApb2enr::usart1en),
    set(RccApb1enr::usart2en),
    set(RccAhbenr::dma1en)
  );
  
  // Systick timer
  apply(
    write(Stk_Load::Reload, 9000 - 1), // 1ms timer
    clear(Stk_Ctrl::Clksource),
    set(Stk_Ctrl::TickInt),
    set(Stk_Ctrl::Enable),
    write(Stk_Priority::pri, mcu::interrupt_priorities(IRQ::systick_irqn))
  );
}
