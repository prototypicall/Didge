#pragma once

#include <string_view>
#include <array>

#include "mcu.hpp"

namespace devices {

  struct step_gen {
    static constexpr uint64_t ClockFreq = mcu::CPU_Clock_Freq_Hz;
    static constexpr uint8_t ClockDiv = 2;

    static constexpr unsigned int min_count = mcu::min_timer_capture_count; // required by timer

    using step_pin = mcu::pins::step_pin;
    using dir_pin = mcu::pins::dir_pin;

    struct start_stop {
      volatile uint16_t cnt_start{}, cnt_stop{};
    };

    struct State {
      start_stop counts_reverse{}, counts_delayed{};
      volatile uint16_t counts_step = 0; // step pulse duration in #timer clock cycles
      volatile bool delayed_pulse = false;
      volatile bool direction = false; // true -> reverse direction
      volatile bool direction_polarity = false; // not inverted
    };

    static State state;

    static void init() {
      using namespace Kvasir;
      //Port
      apply(write(step_pin::cr::mode, gpio::PinMode::Output_2Mhz),
            write(step_pin::cr::cnf, gpio::PinConfig::Output_alternate_push_pull),
            write(dir_pin::cr::mode, gpio::PinMode::Output_2Mhz),
            write(dir_pin::cr::cnf, gpio::PinConfig::Output_push_pull));
      //Timer
      apply(set(Tim3Cr1::opm),
            write(Tim3Psc::psc, ClockDiv - 1),
            write(Tim3Ccmr2Output::oc3m, 0b111), //PWM mode 2
            write(Tim3Smcr::sms, 0b110), // Trigger mode
            write(Tim3Smcr::ts, 0), // ITR0 - tim1
            set(Tim3Ccer::cc3e),
            clear(Tim3Sr::uif),
            set(Tim3Dier::uie)  // enable update interrupt
      );
      mcu::enable_interrupt<IRQ::tim3_irqn>();
    }
    
    static void configure(unsigned int dir_setup_ns, unsigned int step_pulse_ns,
                          bool invert_step, bool invert_dir) {
      apply(write(Kvasir::Tim3Ccer::cc3p, invert_step));
      state.direction_polarity = invert_dir;
      apply(write(dir_pin::odr, state.direction ^ state.direction_polarity));

      constexpr uint64_t TimerFreq = ClockFreq / ClockDiv;
      constexpr uint64_t nanosec = mcu::onesec_in_ns.count();
      //TODO: implement a range checked version
      uint16_t cnt_setup_delay = std::max(min_count,
              1u + static_cast<unsigned int>(dir_setup_ns * TimerFreq / nanosec));
      uint16_t cnt_step = 1u + static_cast<unsigned int> (step_pulse_ns * TimerFreq / nanosec);
      state.counts_reverse = {cnt_setup_delay, 
                              static_cast<uint16_t>(cnt_setup_delay + cnt_step)};
      state.counts_step = cnt_step;
      setup_next_pulse();
    }

    static inline bool get_direction() {
      return state.direction;
    }

    static void change_direction(bool new_dir) {
      state.direction = new_dir;
      using namespace Kvasir;
      apply(write(dir_pin::odr, new_dir ^ state.direction_polarity));
      apply(clear(Tim3Ccmr2Output::oc3fe),
            write(Tim3Ccr3::ccr3, state.counts_reverse.cnt_start),
            write(Tim3Arr::arr, state.counts_reverse.cnt_stop));
    }
    
    static void set_delay(unsigned delay_count) {
      delay_count = delay_count / ClockDiv;
      if (delay_count >= min_count) {
        auto end_delayed = state.counts_step + delay_count;
        if (end_delayed >= std::numeric_limits<uint16_t>::max()) {
          end_delayed = std::numeric_limits<uint16_t>::max() - 1; // clamp
          state.counts_delayed = {
            static_cast<uint16_t>(end_delayed - state.counts_step),
            static_cast<uint16_t>(end_delayed)};
        }
        else {
          state.counts_delayed = {
            static_cast<uint16_t>(delay_count), 
            static_cast<uint16_t>(end_delayed)};
        }
        state.delayed_pulse = true;
      }
      else {
        state.delayed_pulse = false;
      }
    }

    static inline void process_interrupt() {
      apply(clear(Kvasir::Tim3Sr::uif));
      setup_next_pulse();
    }

  private:
    static void setup_next_pulse() {
      using namespace Kvasir;
      if (state.delayed_pulse) {
        apply(clear(Tim3Ccmr2Output::oc3fe),
              write(Tim3Ccr3::ccr3, state.counts_delayed.cnt_start),
              write(Tim3Arr::arr, state.counts_delayed.cnt_stop));
      }
      else {
        apply(set(Tim3Ccmr2Output::oc3fe), // enable fast enable 
              write(Tim3Ccr3::ccr3, 1), // For some reason 0 does not work
              write(Tim3Arr::arr, state.counts_step));
      }
    }
  };

  struct encoder {
    using pin_A = mcu::pins::enc_A;
    using pin_B = mcu::pins::enc_B;
    using pin_Ch3 = mcu::pins::tim1_ch3;

    //TODO: configurable polarity selection and input filter
    static constexpr auto input_filter = 0b0000; //0b0011 for N = 8 samples
    
    using CounterValue = uint16_t;

    static void init() {
      //Pins
      using namespace Kvasir;
      apply(write(pin_A::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            set(pin_A::bsrr),
            write(pin_B::cr::cnf, gpio::PinConfig::Input_pullup_pulldown),
            set(pin_B::bsrr)
      );
      apply(write(pin_Ch3::cr::mode, gpio::PinMode::Output_2Mhz),
            write(pin_Ch3::cr::cnf, gpio::PinConfig::Output_alternate_push_pull)
      );
      //Timer
      apply(write(Tim1Ccmr1Input::cc1s, 0b01),
            write(Tim1Ccmr1Input::ic1f, input_filter),
            write(Tim1Ccmr1Input::cc2s, 0b01),
            write(Tim1Ccmr1Input::ic2f, input_filter),
            write(Tim1Ccmr2Output::oc3m, 0b001), // Set on match
            write(Tim1Smcr::sms, 0b011),
            write(Tim1Cr2::mms, 0b110), // oc3ref is TRGO
            set(Tim1Ccer::cc3e), // enable ch3 output
            set(Tim1Bdtr::moe) // enable outputs
      );
      apply(set(Tim1Cr1::cen));
      
      setup_cc_interrupt();
    }

    static void setup_cc_interrupt() {
      using namespace Kvasir;
      apply(set(Tim1Dier::cc3ie), set(Tim1Dier::cc4ie));
      clear_cc_interrupt();
      mcu::enable_interrupt<IRQ::tim1_cc_irqn>();
    }
    
    static inline bool is_cc_fwd_interrupt() {
      using namespace Kvasir;
      return apply(read(Tim1Sr::cc3if));
    }
    
    static inline void clear_cc_interrupt() {
      using namespace Kvasir;
      apply(clear(Tim1Sr::cc3if),
              clear(Tim1Sr::cc4if));
    }

    static inline void update_channels(CounterValue next, CounterValue prev) {
      using namespace Kvasir;
      apply(write(Tim1Ccr3::ccr3, next),
            write(Tim1Ccr4::ccr4, prev)
      );
    }

    static void inline trigger_clear() {
      apply(write(Kvasir::Tim1Ccmr2Output::oc3m, 0b100)); // force oc3ref low
    }

    static void inline trigger_restore() {
      apply(write(Kvasir::Tim1Ccmr2Output::oc3m, 0b001)); // set oc3ref on match
    }

    static void inline trigger_manual_pulse() {
      apply(write(Kvasir::Tim1Ccmr2Output::oc3m, 0b101)); // Force oc3ref high
      trigger_clear();
      trigger_restore();
    }

    static inline CounterValue get_count() {
      return apply(read(Kvasir::Tim1Cnt::cnt));
    }
  };

  template <uint8_t Period_ms = 10, uint8_t Samples = 16>
  struct rpm_counter {
    static constexpr uint16_t periods_per_min = 60000 / Period_ms;
    static constexpr uint8_t Sampling_period = Period_ms;
    
    volatile inline static uint16_t last_reading = 0;
    volatile inline static uint8_t sample_index = 0;
    volatile inline static unsigned running_sum = 0;
    volatile inline static uint16_t sum = 0;

    static inline uint16_t convert_sample(uint16_t c) {
      auto p = last_reading;
      uint16_t d = (c > p) ? (c - p) : (p - c);
      if (d > std::numeric_limits<uint16_t>::max() / 2) {
        d = std::numeric_limits<uint16_t>::max() - d;
      }
      last_reading = c;
      return d;
    }

    static bool process_sample(uint16_t current_reading) {
      running_sum += convert_sample(current_reading);
      auto i = sample_index;
      if ((++i) == Samples) {
        sum = running_sum;
        sample_index = 0;
        running_sum = 0;
        return true;
      } else {
        sample_index = i;
        return false;
      }
    }

    static uint16_t get_rpm(uint16_t encoder_resolution) {
      auto rpm = (sum * periods_per_min) / (encoder_resolution * Samples);
      return static_cast<uint16_t>(rpm);
    }
  };
  
  struct encoder_pulse_duration  {
    using pin_ch2 = mcu::pins::tim2_ch2;

    static constexpr uint16_t Prescaler = 4; // Period of a single channel contains 4 encoder changes
    
    volatile static inline uint16_t last_full_period = 0;
    
    static void init() {
      using namespace Kvasir;
      // Pin is input floating by default so no action necessary
      // Timer registers - Configure for "PWM Input Mode"
      apply(write(Tim2Psc::psc, Prescaler - 1),
            write(Tim2Ccmr1Input::cc1s, 0b10), // Source input 2
            write(Tim2Ccmr1Input::cc2s, 0b01), // Source input 2
            set(Tim2Ccer::cc2p), // invert polarity
            write(Tim2Smcr::ts, 0b110), // trigger on filtered input 2
            write(Tim2Smcr::sms, 0b100), // Slave mode: reset
            set(Tim2Dier::cc2de), // enable DMA for IC2
            write(Tim2Ccr3::ccr3, std::numeric_limits<uint16_t>::max() - 1), // Timeout
            set(Tim2Dier::cc3ie) // enable compare 3 interrupt
      ); 
      // DMA
      apply(write(Dma1Cpar7::pa, Tim2Ccr2::Addr::value), // Acc. to docs. Ch1 should have
                                                         // the period value but Ch2 does
            write(Dma1Cmar7::ma, reinterpret_cast<unsigned>(std::addressof(last_full_period))),
            write(Dma1Cndtr7::ndt, 1),
            write(Dma1Ccr7::msize, 0b01), // 16 bits
            write(Dma1Ccr7::psize, 0b01), 
            set(Dma1Ccr7::circ) // circular mode -> continuous
      );
      apply(set(Dma1Ccr7::en)); // enable DMA channel
      
      mcu::enable_interrupt<IRQ::tim2_irqn>();

      apply(set(Tim2Ccer::cc1e), set(Tim2Ccer::cc2e), // enable captures
            set(Tim2Cr1::cen)); // enable timer
    }
    
    //TODO: configurable filtering (should be same as encoder timer (Tim1))
    
    static inline void process_interrupt() {
      last_full_period = 0; // time out - too slow
      apply(clear(Kvasir::Tim2Sr::cc3if));
    }
    
    static inline uint16_t last_duration() {
      return last_full_period;
    }
  };
  
  
  template<unsigned ClkFreq = mcu::CPU_Clock_Freq_Hz / 2, unsigned BaudRate = 115200 >
  struct Serial2 {
    using pin_TX = mcu::pins::uart2_TX;
    //using pin_RX = mcu::pins::uart2_RX;

    static void init() {
      //Pins
      using namespace Kvasir;
      apply(write(pin_TX::cr::mode, gpio::PinMode::Output_2Mhz),
            write(pin_TX::cr::cnf, gpio::PinConfig::Output_alternate_push_pull));
      apply(write(Usart2Brr::brr_12_4, mcu::usart_brr_val(ClkFreq, BaudRate)),
            set(Usart2Cr1::te));
      apply(set(Usart2Cr1::ue));
    }

    static void put_char(char c) {
      using namespace Kvasir;
      while (!apply(read(Usart2Sr::txe)));
      apply(write(Usart2Dr::dr, c));
    }

  };

}
