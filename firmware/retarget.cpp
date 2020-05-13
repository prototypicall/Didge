#include "devices.hpp"

extern "C" {

  int _write (int fd, char *ptr, int len)
  {
    /* Write "len" of char from "ptr" to file id "fd"
     * Return number of char written.
     * Need implementing with UART here. */
    for (; len > 0; --len, ++ptr) {
      devices::Serial2<>::put_char(*ptr);
    }
    return len;
  }

  int _read (int fd, char *ptr, int len)
  {
    /* Read "len" of char to "ptr" from file id "fd"
     * Return number of char read.
     * Need implementing with UART here. */
    return len;
  }

  void _ttywrch(int ch) {
    devices::Serial2<>::put_char(ch);
  }

}