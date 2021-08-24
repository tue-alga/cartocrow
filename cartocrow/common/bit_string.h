/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#ifndef CARTOCROW_COMMON_BIT_STRING_H
#define CARTOCROW_COMMON_BIT_STRING_H

#include <limits>


namespace cartocrow
{

template<typename bits_t, typename bit_size_t = int>
class BitStr
{
 public:
  static bool CheckFit(const bit_size_t& bit) { return bit < std::numeric_limits<bits_t>::digits; }

  inline static BitStr FromBit(const bit_size_t& bit) { return BitStr(ToString(bit)); }

  inline static BitStr FromString(const bits_t& string) { return BitStr(string); }

  BitStr() : bits(0) {}

  inline bool IsEmpty() const { return bits == 0; }

  inline bool Overlaps(const BitStr& string) const { return (string.bits & bits) != 0; }

  inline const bits_t& Get() const { return bits; }

  inline bool operator[](const bit_size_t& bit) const { return (ToString(bit) & bits) != 0; }

  inline BitStr operator+(const bit_size_t& bit) const { return BitStr(bits | ToString(bit)); }

  inline BitStr operator-(const bit_size_t& bit) const { return BitStr(bits & ~ToString(bit)); }

  inline BitStr& operator+=(const bit_size_t& bit) { bits |= ToString(bit); return *this; }

  inline BitStr& operator-=(const bit_size_t& bit) { bits &= ~ToString(bit); return *this; }

  inline BitStr operator+(const BitStr& string) const { return BitStr(bits | string.bits); }

  inline BitStr operator-(const BitStr& string) const { return BitStr(bits & ~string.bits); }

  inline BitStr operator&(const BitStr& string) const { return BitStr(bits & string.bits); }

  inline BitStr operator^(const BitStr& string) const { return BitStr(bits ^ string.bits); }

  inline BitStr& operator+=(const BitStr& string) { bits |= string.bits; return *this; }

  inline BitStr& operator-=(const BitStr& string) { bits &= ~string.bits; return *this; }

  inline BitStr& operator&=(const BitStr& string) { bits &= string.bits; return *this; }

  inline BitStr& operator^=(const BitStr& string) { bits ^= string.bits; return *this; }

 private:
  inline static bits_t ToString(const bit_size_t& bit) { return 1 << bit; }

  explicit BitStr(const bits_t& string) : bits(string) {}

  bits_t bits;
}; // class BitStr

using BitString_16 = BitStr<unsigned short>;
using BitString_32 = BitStr<unsigned int>;
using BitString_64 = BitStr<unsigned long>;
using BitString_128 = BitStr<unsigned long long>;

using BitString = BitString_32;

} // namespace cartocrow

#endif //CARTOCROW_COMMON_BIT_STRING_H
