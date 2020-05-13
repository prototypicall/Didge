#pragma once

#include <cstdio>

#include <string_view>
#include <array>
#include <string>
#include <algorithm>
#include <type_traits>

#include <boost/rational_minimal.hpp>
#include <numeric>

namespace threads {
  //using ratio_t = boost::rational<uint64_t>;
  using Rational = boost::rational<unsigned>;

  constexpr auto tpi_pitch = [](auto tpi) -> Rational {
    return {127, 5 * tpi};
  };
  
  enum class pitch_type {
    mm,
    tpi,
  };
  
  struct pitch_info {
    std::string_view pitch_str;
    Rational value;
    pitch_type type;
    
    std::string_view unit() const {
      if (type == pitch_type::mm)
        return "mm";
      else
        return "tpi";
    }
  };

  struct thread {
    std::string_view name;
    pitch_info pitch;
    bool is_custom{false};
    
    char * description_c_str(char *buf) const {
      buf = std::copy(name.begin(), name.end(), buf);
      *buf++ = ' ';
      buf = std::copy(pitch.pitch_str.begin(), pitch.pitch_str.end(), buf);
      auto unit = pitch.unit();
      buf = std::copy(unit.begin(), unit.end(), buf);
      if (is_custom) {
        *buf++ = '*';
      }
      return buf;
    }
  };

  namespace detail {
    // some stdlib functionality is either not constexpr or uses too much space
    constexpr unsigned sv_to_unsigned(std::string_view sv) {
      unsigned result = 0;
      for (auto i = sv.begin(); i != sv.end(); ++i) {
        uint8_t digit = *i - '0';
        result = result * 10 + digit;
      }
      return result;
    }

    constexpr unsigned pow10(unsigned power) {
      unsigned result = 1;
      for (; power > 0; --power) { result *= 10; }
      return result;
    }

    // Convert string containing decimal into rational number with no error
    // e.g. "1.865" -> 373/200
    constexpr Rational decimal_to_rational(std::string_view decimal) {
      auto i_dot = decimal.find('.');
      if (i_dot == decimal.npos) {
        return {sv_to_unsigned(decimal), 1};
      } else {
        auto fraction = decimal;
        fraction.remove_prefix(i_dot + 1);
        auto denom = pow10(fraction.size());
        decimal.remove_suffix(decimal.size() - i_dot);
        auto num = sv_to_unsigned(decimal) * denom + sv_to_unsigned(fraction);
        return {num, denom};
      }
    }
  }

  inline namespace literals {
    constexpr pitch_info operator"" _mm(const char* pitch) {
      return {pitch, detail::decimal_to_rational(pitch), pitch_type::mm};
    }

    constexpr pitch_info operator"" _tpi(const char* pitch) {
      auto r = detail::decimal_to_rational(pitch);
      // TPI = 1 inch/#Threads : 1 inch=25.4mm = 127 / 5 mm
      return {pitch, {127 * r.denominator(), 5 * r.numerator()}, pitch_type::tpi};
    }
  }

}