#pragma once

#include <optional>
#include <array>
#include <cstdio>

#include "mcu.hpp"
#include "circular_buffer.hpp"

#include "thread_list.hpp"
#include "common.hpp"

namespace devices {
  
  template<unsigned ClkFreq = mcu::CPU_Clock_Freq_Hz, unsigned BaudRate = 115200 >
  struct hmi {

    enum class hmi_event : uint8_t {
      none = 0,
      hmi_ready,
      btn_thread_minus,
      btn_thread_plus,
      btn_thread_select,
      btn_menu,
      btn_settings,
      btn_p3_cancel,
      btn_p3_ok,
      btn_p3_pgup,
      btn_p3_pgdn,
      hs_p3_grid,
      value_unsigned
    };
    
    //TODO: usarts need a generic interface 
    using pin_TX = mcu::pins::uart1_remapped_TX;
    using pin_RX = mcu::pins::uart1_remapped_RX;

    static void init() {
      using namespace Kvasir;
      apply(set(Kvasir::AfioMapr::usart1Remap));
      apply(write(pin_TX::cr::mode, gpio::PinMode::Output_2Mhz),
            write(pin_TX::cr::cnf, gpio::PinConfig::Output_alternate_push_pull),
            write(pin_RX::cr::mode, gpio::PinMode::Input),
            write(pin_RX::cr::cnf, gpio::PinConfig::Input_floating));
      apply(write(Usart1Brr::brr_12_4, mcu::usart_brr_val(ClkFreq, BaudRate)),
            set(Usart1Cr1::rxneie),
            set(Usart1Cr1::re),
            set(Usart1Cr1::te));
      apply(set(Usart1Cr1::ue));
    }

    static inline void process_interrupt() {
      using namespace Kvasir;
      auto c = apply(read(Usart1Dr::dr));
      read_buf.push(c);
      if (c == 0xFF) {
        if ((++decode_state.ff_count) == 3) {
          decode_state.ff_count = 0;
          ++decode_state.received_packets;
        }
      } else {
        decode_state.ff_count = 0;
      }
    }

    static inline hmi_event process() {
      if (decode_state.read_packets != decode_state.received_packets) {
        return process_new_packet();
      }
      else return hmi_event::none;
    }

    static void connect(bool wait_for_sync) {
      apply(read(Kvasir::Usart1Dr::dr)); // clear chars if any
      mcu::enable_interrupt<Kvasir::IRQ::usart1_irqn>();
      reset_hmi();
      if (wait_for_sync) {
        while (process() != hmi_event::hmi_ready);
      }
    }

    static void send_rpm(uint16_t val) {
      send_packet(std::sprintf(out_buf.begin(), "n0.val=%u", val));
    }
    
    static void send_thread_info(const threads::thread& thr) {
      auto b = out_buf.begin();
      auto i = sprintf(b, "t0.txt=\"");
      auto e = thr.description_c_str(b + i);
      *e++ = '"';
      send_packet(e - b);
    }
    
    // A hacky implementation for a *listbox* like UI using buttons
    template <typename ThreadValidation>
    static int16_t select_thread(int16_t selected_index, ThreadValidation f_thread) {
      constexpr uint8_t num_rows = 7;
      
      auto f_get_thread = [](int16_t index) {
        std::optional<threads::thread> result;
        if (index < threads::pitch_list_size) {
          result = threads::pitch_list[index];
        }
        return result;
      };

      auto f_draw_selection = [&](int16_t row, bool selected) {
        send_packet(std::sprintf(out_buf.begin(), "r%u.val=%u", row, (unsigned)selected));
      };
      
      auto f_draw_grid = [&](int16_t top_index) {
        for (uint8_t r = 0; r < num_rows; ++r) {
          // write thread info
          auto thr = f_get_thread(top_index + r);
          auto i = std::sprintf(out_buf.begin(), "r%u.txt=\"", r);
          auto b = out_buf.begin() + i;
          if (thr) {
            auto e = thr->description_c_str(b);
            i += e - b;
          }
          out_buf[i++] = '"';
          send_packet(i);
          send_packet(std::sprintf(out_buf.begin(), "vis r%u,%u", r, (thr)));
          f_draw_selection(r, (thr) && ((top_index + r) == selected_index));
        }
      };
      
      select_page(2);
      
      uint8_t list_page = selected_index / num_rows;
      uint8_t num_list_pages = (threads::pitch_list_size + num_rows - 1) / num_rows;
      uint16_t r_offset =  list_page * num_rows;
      f_draw_grid(r_offset);

      auto f_show_page_no = [num_list_pages](int16_t page) {
        send_packet(std::sprintf(out_buf.begin(), "t1.txt=\"%d\\r/\\r%d\"", 
                1 + page, num_list_pages));
      };
      f_show_page_no(list_page);
      
      auto save_selected_index = selected_index;
      for (auto done = false; !done; ) {
        auto e = process();
        switch (e) {
          case hmi_event::btn_p3_cancel:
            selected_index = save_selected_index;
            done = true;
            break;
          case hmi_event::btn_p3_ok:
            {
              auto err = f_thread(selected_index);
              if (err == 0) {
                done = true;
              }
              else {
                send_packet(std::sprintf(out_buf.begin(), "err.val=%u", err));
                send_string_packet("click t2,0");
                send_string_packet("click t2,1");
              }
            }
            break;
          case hmi_event::btn_p3_pgdn:
            if (r_offset + num_rows < threads::pitch_list_size) {
              r_offset += num_rows;
              f_draw_grid(r_offset);
              f_show_page_no(++list_page);
            }
            break;
          case hmi_event::btn_p3_pgup:
            if (r_offset - num_rows >= 0) {
              r_offset -= num_rows;
              f_draw_grid(r_offset);
              f_show_page_no(--list_page);
            }
            break;
          case hmi_event::hs_p3_grid: // click to select
            {
              auto row = last_range_id;
              auto new_index = r_offset + row;
              if ((selected_index >= r_offset) && (selected_index < (r_offset + num_rows))) {
                f_draw_selection(selected_index - r_offset, (new_index == selected_index));
              }
              selected_index = new_index;
            }
            break;
        }
      }
      
      select_page(0);
      return selected_index;
    }
    
    static void select_page(uint16_t page_no) {
      send_packet(std::sprintf(out_buf.begin(), "page %u", page_no));
    }
    
  private:
    inline static container::fixed_size_circular_buffer<uint8_t, 256, uint16_t> read_buf{};
    inline static std::array<char, 256> out_buf{};

    struct Decoding {
      volatile uint16_t ff_count = 0;
      volatile uint8_t received_packets = 0;
      volatile uint8_t read_packets = 0;
    };

    struct Point {
      uint16_t x, y;
    };
    
    inline static Decoding decode_state{};
    inline static volatile bool connected = false;
    inline static volatile unsigned last_unsigned = 0;
    inline static volatile unsigned last_range_id = 0;

    static constexpr std::string_view message_suffix{"\xFF\xFF\xFF"};

    static hmi_event process_new_packet() {
      // https://nextion.tech/instruction-set/
      auto c = read_buf.pop();
      uint8_t skip_count = 3; // 3 bytes for message suffix
      hmi_event result = hmi_event::none;
      switch (c) {
        case 0x0: // startup
          skip_count = 5;
          break;
        case 0x65: //touch event
          result = process_press(read_buf.pop(), read_buf.pop());
          skip_count = 4; // ignore press/release
          break;
        case 0x88:
          result = hmi_event::hmi_ready;
          connected = true;
          break;
        case 0x71: // numeric data
          last_unsigned = read_buf.pop() + (read_buf.pop() << 8) +
                  (read_buf.pop() << 16) + (read_buf.pop() << 24);
          result = hmi_event::value_unsigned;
          break;
        default: //ignored message
          for (auto b = read_buf.pop(); b != '\xFF'; b = read_buf.pop());
          skip_count = 2;
      }
      if (skip_count > 0) {
        read_buf.skip(skip_count);
      }
      ++decode_state.read_packets;
      return result;
    }

    static hmi_event process_press(uint8_t page, uint8_t id) {
      if (page == 0) {
        switch (id) {
          case 5: return hmi_event::btn_thread_select;
          case 6: return hmi_event::btn_thread_minus;
          case 7: return hmi_event::btn_thread_plus;
          case 17: return hmi_event::btn_menu;
          default:
            return hmi_event::none;
        }
      }
      else if (page == 2) {
        switch (id) {
          case 2: return hmi_event::btn_p3_cancel;
          case 3: return hmi_event::btn_p3_ok;
          case 8: return hmi_event::btn_p3_pgup;
          case 9: return hmi_event::btn_p3_pgdn;
          case 14 ... 20: // gcc extension
            last_range_id = id - 14;
            return hmi_event::hs_p3_grid;
          default:
            return hmi_event::none;
        }        
      }
      else {
        return hmi_event::none;        
      }
    }
    
    static void draw_box(const Point& p1, const Point& p2, unsigned color) {
      send_packet(std::sprintf(out_buf.begin(), "draw %u,%u,%u,%u,%u", 
              p1.x, p1.y, p2.x, p2.y, color));
    }
    
    static void reset_hmi() {
      send_string_packet("rest");
    }
    
    static void send_packet(int num_chars) {
      for (auto i = out_buf.begin(); num_chars > 0; --num_chars, ++i) {
        send_char(*i);
      }
      for (char c : message_suffix) {
        send_char(c);
      }
    }
    
    static inline void send_char(char c) {
      using namespace Kvasir;
      while (!apply(read(Usart1Sr::txe)));
      apply(write(Usart1Dr::dr, c));
    }

    static void send_string_packet(const std::string_view& s) {
      for (char c : s) {
        send_char(c);
      }
      for (char c : message_suffix) {
        send_char(c);
      }
    }
    
    static unsigned get_unsigned_value(const char* descriptor) {
      for (char c : std::string_view("get ")) { send_char(c); }
      send_string_packet(descriptor);
      for (auto e = process(); e != hmi_event::value_unsigned; e = process());
      return last_unsigned;
    }
  };  
  
}