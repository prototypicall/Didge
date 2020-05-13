#pragma once

#include <chrono>

#include <Chip/STM32F103xx.hpp>
#include <Register/Register.hpp>

namespace mcu {
  // Work in progress
  constexpr uint64_t CPU_Clock_Freq_Hz = 72 * std::mega::num;
  
  using namespace std::chrono_literals;
  constexpr auto onesec_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(1s);
  
  constexpr uint16_t min_timer_capture_count = 5;
  
  constexpr uint16_t usart_brr_val(uint64_t usart_clock_freq, uint32_t baud_rate) {
    return usart_clock_freq/ baud_rate;
  }
  
  namespace pins {
    using led_pin = Kvasir::gpio::Pin<Kvasir::gpio::PC, 13>;
    using debug_pin = Kvasir::gpio::Pin<Kvasir::gpio::PB, 12>; 
    
    using step_pin = Kvasir::gpio::Pin<Kvasir::gpio::PB, 0>;
    using dir_pin = Kvasir::gpio::Pin<Kvasir::gpio::PB, 1>;

    using enc_A = Kvasir::gpio::Pin<Kvasir::gpio::PA, 8>;
    using enc_B = Kvasir::gpio::Pin<Kvasir::gpio::PA, 9>;
    using tim1_ch3 = Kvasir::gpio::Pin<Kvasir::gpio::PA, 10>;

    using tim2_ch2 = Kvasir::gpio::Pin<Kvasir::gpio::PA, 1>;
    
    using uart2_TX = Kvasir::gpio::Pin<Kvasir::gpio::PA, 2>;
    //using uart2_RX = Kvasir::gpio::Pin<Kvasir::gpio::PA, 3>;

    using uart1_remapped_TX = Kvasir::gpio::Pin<Kvasir::gpio::PB, 6>; 
    using uart1_remapped_RX = Kvasir::gpio::Pin<Kvasir::gpio::PB, 7>;
  }
  
  // Static (i.e. compile time) map of used IRQs and their values
  // if you are getting a compile time error, you might be missing an entry.
  constexpr uint8_t interrupt_priorities(Kvasir::nvic::irq_number_t irq) {
    switch (irq) {
      case Kvasir::IRQ::tim2_irqn:    return 1;
      case Kvasir::IRQ::tim1_cc_irqn: return 2;
      case Kvasir::IRQ::tim3_irqn:    return 4;
      case Kvasir::IRQ::usart1_irqn:  return 6;
      case Kvasir::IRQ::systick_irqn: return 15;
    }
  };
  
  template <Kvasir::nvic::irq_number_t irq_n>
  inline void enable_interrupt() {
    using irq = Kvasir::nvic::irq<irq_n>;
    apply(
      write(irq::setena, true),
      write(irq::ipr, mcu::interrupt_priorities(irq_n))
    );
  }
  
  static void toggle_led() {
    bool led = apply(read(pins::led_pin::odr)); 
    apply(write(pins::led_pin::odr, !led));
  }
  
  static void init() {
    using namespace Kvasir;
    apply(write(pins::led_pin::cr::mode, gpio::PinMode::Output_2Mhz));

    apply(write(pins::debug_pin::cr::mode, gpio::PinMode::Output_2Mhz),
          write(pins::debug_pin::cr::cnf, gpio::PinConfig::Output_push_pull));
    
  }
  
  inline volatile unsigned int milliseconds = 0;

  inline void delay_ms(unsigned int ms) {
    const auto target_tick = milliseconds + ms;
    while (milliseconds != target_tick);
  }
  

}

