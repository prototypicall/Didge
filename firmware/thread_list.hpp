#pragma once

#include "threads.hpp"

namespace threads {

  // A temporary short list of threads. Currently implemented as a const array
  // TODO: 
  // * complete the list (source?) and add support for reading/writing on flash memory
  // * add support for filtering (wrt unit type, etc.)
  const thread pitch_list[] = {
    {"R1:1", 15_tpi, true}, // same pitch as demo leadscrew definition -> one to one rotation ratio
    {"1/4 ", 20_tpi},
    {"1/4 ", 28_tpi},
    {"5/16", 18_tpi},
    {"5/16", 24_tpi},
    {"3/8 ", 16_tpi},
    {"3/8 ", 24_tpi},
    {"Huge!", 4_tpi},
    {"M1.6", .35_mm},
    {"M2 ", 0.40_mm},
    {"M2.5", .45_mm},
    {"M3 ", 0.50_mm},
    {"M4 ", 0.70_mm},
    {"M5 ", 0.80_mm},
    {"M6 ", 1.00_mm},
    {"M8 ", 1.25_mm},
    {"M10", 1.50_mm},
    {"M12", 1.75_mm}
  };

  const int16_t pitch_list_size = std::extent<decltype(pitch_list)>::value;

  constexpr int16_t default_pitch_index = 0;

}