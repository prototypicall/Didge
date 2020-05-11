#pragma once 
#include <Register/Utility.hpp>
namespace Kvasir {
//General purpose I/O
  namespace gpio {

    enum Port : unsigned int {
      PA = 0x40010800u,
      PB = 0x40010C00u,
      PC = 0x40011000u,
      PD = 0x40011400u,
      PE = 0x40011800u
    };

    // Moved outside of cr register for convenience
    enum class PinMode {
      Input = 0b00,
      Output_10Mhz = 0b01,
      Output_2Mhz = 0b10,
      Output_50Mhz = 0b11
    };

    enum class PinConfig {
      Input_analog = 0b00,
      Input_floating = 0b01,
      Input_pullup_pulldown = 0b10,
      Output_push_pull = 0b00,
      Output_open_drain = 0b01,
      Output_alternate_push_pull = 0b10,
      Output_alternate_open_drain = 0b11
    };
    
    template <Port port_addr, unsigned pin_no>
    struct Pin {
      static_assert(pin_no < 16, "Pin can be 0-15!");

      struct cr { // Control register
        static constexpr unsigned int addr_offs = (pin_no > 7) ? 4 : 0;
        static constexpr unsigned int bit_offs = (pin_no & 0x7) << 2;
        
        static constexpr Register::FieldLocation<
          Register::Address<port_addr + addr_offs>, 
          Register::maskFromRange(bit_offs + 3, bit_offs + 2), 
          Register::ReadWriteAccess, PinConfig> cnf{};
          
        static constexpr Register::FieldLocation<
          Register::Address<port_addr + addr_offs>,
          Register::maskFromRange(bit_offs + 1, bit_offs), 
          Register::ReadWriteAccess, PinMode> mode{};
      };
      
      static constexpr Register::FieldLocation<Register::Address<port_addr + 0x8>, 
        (1 << pin_no), Register::ReadOnlyAccess, unsigned> idr{};
        
      static constexpr Register::FieldLocation<Register::Address<port_addr + 0xC>, 
        (1 << pin_no), Register::ReadWriteAccess, unsigned> odr{};
      
      static constexpr Register::FieldLocation<Register::Address<port_addr + 0x10>, 
        (1 << pin_no), Register::WriteOnlyAccess, unsigned> bsrr{};

      static constexpr Register::FieldLocation<Register::Address<port_addr + 0x14>, 
        (1 << pin_no), Register::WriteOnlyAccess, unsigned> brr{};
    };
    
  }
}
