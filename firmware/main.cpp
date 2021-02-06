#include <limits>
#include <algorithm>

// Kvasir imports
#include <Chip/STM32F103xx.hpp>
#include <Register/Register.hpp>
#include <Register/Utility.hpp>

#include "mcu.hpp"
#include "devices.hpp"
#include "hmi.hpp"
#include "gear.hpp"
#include "threads.hpp"
#include "thread_list.hpp"
#include "configuration.hpp"
#include "common.hpp"


namespace systick_state {
  volatile uint8_t rpm_sample_prescale_count = 0;
}

namespace ui {
  volatile bool rpm_update = false;
  volatile bool rpm_report = false;
  util::cached_value<uint16_t> rpm_cached{};
}

namespace control {
  
  enum class State : uint8_t {
    stopped,
    in_sync,
    ramping,
  };
  
  volatile State state = State::in_sync; // TODO: default should be OFF
}

extern "C" { // interrupt handlers
  void SysTick_Handler() { // Called every 1 ms
    using rpm_sampler = devices::rpm_counter<>;
    using namespace systick_state;
    if (ui::rpm_report) {
      auto psc = rpm_sample_prescale_count;
      if (++psc == rpm_sampler::Sampling_period) {
        if (rpm_sampler::process_sample(devices::encoder::get_count())) {
          ui::rpm_update = true;
        }
        rpm_sample_prescale_count = 0;
      }
      else {
        rpm_sample_prescale_count = psc;
      }
    }  
    if (((++mcu::milliseconds) & 1023) == 0) {
      mcu::toggle_led();
    }
  }

  void TIM1_CC_IRQHandler() {
    using namespace devices;
    auto enc = encoder::get_count();
    bool dir = step_gen::get_direction();
    const bool fwd = encoder::is_cc_fwd_interrupt();
    encoder::clear_cc_interrupt();
    using namespace gear;
    if (fwd) {
      encoder::trigger_clear();
      state.err = range.next.error;
      range.next_jump(dir, enc);
      step_gen::set_delay(phase_delay(encoder_pulse_duration::last_duration(), range.next.error));
      encoder::trigger_restore();
    }
    else { // Change direction, setup delayed pulse and do manual trigger     
      dir = !dir;
      step_gen::change_direction(dir);
      encoder::trigger_manual_pulse();
      state.err = range.prev.error;
      range.next_jump(dir, enc);
    }
    encoder::update_channels(range.next.count, range.prev.count);
  }

  void TIM2_IRQHandler() {
    devices::encoder_pulse_duration::process_interrupt();
  }

  void TIM3_IRQHandler() {
    using devices::step_gen;
    step_gen::process_interrupt();
    gear::state.output_position += step_gen::get_direction() ? 1 : -1;
  }

  void USART1_IRQHandler() {
    devices::hmi<>::process_interrupt();
  }
} // extern "C"

Configuration config{};


void change_thread() {
  // TODO:
  // Acceleration:
  // find out current and target output speed
  //   check for target speed too high -> trigger stop
  //      if speeds are not significantly different do nothing
  //   if the state is stopped current speed is zero, 
  //    if ramping -> re-adjust target speed
  // setup acceleration settings in acceleration device
  // switch step_gen to trigger from the accelerator
  auto pr = config.calculate_ratio();
  gear::configure(pr, devices::encoder::get_count());
  devices::hmi<>::send_thread_info(config.thread);
}

int main() {
  using namespace devices;

  mcu::init();
  
  //Serial2<>::init(); // Used as console
  
  step_gen::init();
  step_gen::configure(config.step_dir_hold_ns, config.step_pulse_ns, 
          config.invert_step_pin, config.invert_dir_pin);
  
  gear::configure(config.calculate_ratio(), 0);

  encoder::init();
  encoder::update_channels(gear::range.next.count, gear::range.prev.count);
  
  encoder_pulse_duration::init();
  
  using display = hmi<>;
  display::init();
  display::connect(false); // wait_for_sync : false for PC sim., true for actual display
  mcu::delay_ms(200);
  
  ui::rpm_report = true;
  
  display::send_thread_info(config.thread);
  
  auto f_check_thread = [&](int16_t index) -> uint8_t {
    return config.verify_thread<encoder::CounterValue>(index);
  };
  
  while (true) {
    if (ui::rpm_update && ui::rpm_report) {
      ui::rpm_update = false;
      ui::rpm_cached = rpm_counter<>::get_rpm(config.encoder_resolution);
      ui::rpm_cached.on_change(display::send_rpm);
    }
    auto event = display::process();
    if (event != display::hmi_event::none) {
      switch (event) {
        case display::hmi_event::btn_thread_minus:
        case display::hmi_event::btn_thread_plus: 
          config.cycle_thread(event == display::hmi_event::btn_thread_plus);
          change_thread();
          break;
        case display::hmi_event::btn_thread_select:
          ui::rpm_report = false;
          config.select_thread(display::select_thread(config.pitch_list_index, f_check_thread));
          change_thread();
          ui::rpm_report = true;
          break;
          
        //TODO: handle other events
      }
    }
  }
  
  return 0;
}
