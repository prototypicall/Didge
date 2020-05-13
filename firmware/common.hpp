#pragma once

#include <optional>

namespace util {
  
  template <typename PodType>
  class cached_value {
    volatile PodType master{};
    std::optional<PodType> copy{};
  public:
    cached_value& operator= (PodType new_value) {
      master = new_value;
      return *this;
    }
    
    template <typename Func>
    void on_change(Func f) {
      PodType value = master;
      if (copy != value) {
        copy = value;
        f(value);
      }
    }
  };


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
  // C/C++ arithmetic operations are promoted to integral types by default
  // which makes shorter integer operations emit warning messages.
  // These meta-functions provide a warning free path to do arithmetic operations
  // on shorter types. Motivation is that the instructions for shorter integers
  // are more efficient (instructions sizes are smaller).
  // Assumes overflow/underflow does not occur or matter!
  template <typename T>
  constexpr T narrow(unsigned value) {
    return static_cast<T>(value);
  }
  
  template <typename T>
  constexpr T narrow(int value) {
    return static_cast<T>(value);
  }
  
#pragma GCC diagnostic pop
  
}