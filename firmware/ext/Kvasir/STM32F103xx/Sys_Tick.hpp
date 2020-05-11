#pragma once 
#include <Register/Utility.hpp>
namespace Kvasir {
//Reset and clock control
    namespace Stk_Ctrl {
      using Addr = Register::Address<0xE000E010>;
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(16, 16), Register::ReadOnlyAccess,
        unsigned> Countflag{};
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(2, 2), Register::ReadWriteAccess> Clksource{};
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(1, 1), Register::ReadWriteAccess> TickInt{};
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(0, 0), Register::ReadWriteAccess> Enable{};
    }
    namespace Stk_Load {
      using Addr = Register::Address<0xE000E014>;
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(23, 0), Register::ReadWriteAccess> Reload{};
    }
    namespace Stk_Val {
      using Addr = Register::Address<0xE000E018>;
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(23, 0), Register::ReadWriteAccess> Current{};
    }
    /*
    namespace Stk_Calib {
      using Addr = Register::Address<0xE000E01C>;
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(23, 0), Register::ReadWriteAccess> Tenms{};
      //Missing NOREF and SKEW
    }
    */
    namespace Stk_Priority {
      using Addr = Register::Address<0xE000ED20>;
      constexpr Register::FieldLocation<Addr, Register::maskFromRange(31, 28), Register::ReadWriteAccess> pri{};
    }
}
