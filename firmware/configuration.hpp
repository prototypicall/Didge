#pragma once

#include <optional>
#include "threads.hpp"
#include "thread_list.hpp"

struct Configuration {
  using gearing_ratio_t = std::pair<uint16_t, uint16_t>;
  
  uint16_t encoder_resolution{2400u};
  gearing_ratio_t encoder_gearing{1, 1};
  
  uint16_t stepper_full_steps{200u};
  uint16_t stepper_micro_steps{8};
  gearing_ratio_t stepper_gearing{1, 1};
  
  unsigned step_pulse_ns{1200};
  unsigned step_dir_hold_ns{400};
  bool invert_step_pin{false};
  bool invert_dir_pin{true};
  
  using Rational = threads::Rational;
  
  Rational leadscrew_pitch{threads::tpi_pitch(15)};

  threads::thread thread = threads::pitch_list[threads::default_pitch_index];
  int16_t pitch_list_index = threads::default_pitch_index;
  
  Configuration() {
    rationals.encoder = {encoder_resolution * encoder_gearing.first, encoder_gearing.second};
    rationals.steps_per_rev = {stepper_full_steps * stepper_micro_steps *
      stepper_gearing.first, stepper_gearing.second};
  }
  
  Rational calculate_ratio() const {
    return calculate_ratio_for_pitch(thread.pitch.value);
  }
  
  void cycle_thread(bool fwd) {
    //TODO: skip incompatible threads
    if (fwd) {
      auto i = pitch_list_index + 1;
      if (i == threads::pitch_list_size) i = 0;
      pitch_list_index = i;
    }
    else {
      auto i = pitch_list_index - 1;
      if (i < 0) i = threads::pitch_list_size - 1;
      pitch_list_index = i;
    }
    thread = threads::pitch_list[pitch_list_index];
  }
  
  void select_thread(int16_t new_pitch_index) {
    thread = threads::pitch_list[pitch_list_index = new_pitch_index];
  }
  
  enum thread_compatibility : uint8_t {
    thread_OK = 0,
    thread_too_large,
    thread_too_small
  };
  
  template <typename TimerCounter = uint16_t>
  thread_compatibility verify_thread(int16_t thread_index) const {
    auto ratio = calculate_ratio_for_pitch(threads::pitch_list[thread_index].pitch.value);
    auto n = ratio.numerator(), d = ratio.denominator();
    if (n >= d) {
      return thread_too_large;
    }
    if (((d + n - 1) / n) > (std::numeric_limits<TimerCounter>::max() / 2)) {
      return thread_too_small;
    }
    return thread_OK;
  }

private:
  struct Bundle {
    Rational encoder{};
    Rational steps_per_rev{};
  };
  
  Bundle rationals{};

  Rational calculate_ratio_for_pitch(const Rational& pitch) const {
    return (pitch / leadscrew_pitch) * rationals.steps_per_rev / rationals.encoder;
  }
  
};

