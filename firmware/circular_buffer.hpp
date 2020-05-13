#pragma once

#include <array>

namespace container {
  
  template <typename Data, unsigned Size, typename Index>
  class fixed_size_circular_buffer {
    static_assert((Size & (Size - 1)) == 0, "Need power of 2 size");
    
    static constexpr unsigned Mask = Size - 1;
    std::array<Data, Size> buffer{};
    volatile Index read_index = 0;
    volatile Index write_index = 0;
    
  public:
    void push(Data data) {
      buffer[write_index & Mask] = data;
      ++write_index;
    }
    
    Data pop() {
      auto result = buffer[read_index & Mask];
      ++read_index;
      return result;
    }
    
    bool empty() const {
      return read_index == write_index;
    }
    
    void skip(Index count) {
      read_index += count;
    }
  };
  
  
}