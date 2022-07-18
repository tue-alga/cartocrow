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

#ifndef CARTOCROW_CORE_BIT_STRING_H
#define CARTOCROW_CORE_BIT_STRING_H

#include <cstdint>
#include <limits>

namespace cartocrow::necklace_map {

namespace detail {

/// An index-accessible string (or array) of bits.
/**
 * @tparam bits_t The bit string storage type. This must be an integer type.
 * @tparam bit_size_t The type used to access the bits.
 */
template <typename bits_t, typename bit_size_t = int> class BitStr {
  public:
	/// Checks whether the bit string is large enough to fit a specific bit.
	static bool checkFit(const bit_size_t& bit) {
		return bit < std::numeric_limits<bits_t>::digits;
	}

	/// Constructs a new bit string in which only the bit at the given index is
	/// `1`.
	inline static BitStr fromBit(const bit_size_t& bit) {
		return BitStr(toString(bit));
	}

	/// Constructs a bit string from the given string of bits.
	inline static BitStr fromString(const bits_t& string) {
		return BitStr(string);
	}

	/// Constructs a new bit string in which all bits are set to `0`.
	BitStr() : m_bits(0) {}

	/// Checks if all bits in this bit string are `0`.
	inline bool isEmpty() const {
		return m_bits == 0;
	}

	/// Checks if this bit string shares any `1` bits with the given bit string.
	inline bool overlaps(const BitStr& string) const {
		return (string.m_bits & m_bits) != 0;
	}

	/// Returns the string of bits represented by this bit string.
	inline const bits_t& get() const {
		return m_bits;
	}

	/// Returns the value of the bit with the given index.
	inline bool operator[](const bit_size_t& bit) const {
		return (toString(bit) & m_bits) != 0;
	}

	/// Returns a copy of this bit string with the bit at the given index set
	/// to `1`.
	inline BitStr operator+(const bit_size_t& bit) const {
		return BitStr(m_bits | toString(bit));
	}

	/// Returns a copy of this bit string with the bit at the given index set
	/// to `0`.
	inline BitStr operator-(const bit_size_t& bit) const {
		return BitStr(m_bits & ~toString(bit));
	}

	/// Sets the bit at the given index to `1` and returns the result.
	inline BitStr& operator+=(const bit_size_t& bit) {
		m_bits |= toString(bit);
		return *this;
	}

	/// Sets the bit at the given index to `0` and returns the result.
	inline BitStr& operator-=(const bit_size_t& bit) {
		m_bits &= ~toString(bit);
		return *this;
	}

	/// Performs a logical OR with the given bit string.
	inline BitStr operator+(const BitStr& string) const {
		return BitStr(m_bits | string.m_bits);
	}

	/// Performs a logical AND with the negation of the given bit string (i.e.,
	/// sets the bits to `0` that are `1` in the given bit string).
	inline BitStr operator-(const BitStr& string) const {
		return BitStr(m_bits & ~string.m_bits);
	}

	/// Performs a logical AND with the given bit string.
	inline BitStr operator&(const BitStr& string) const {
		return BitStr(m_bits & string.m_bits);
	}

	/// Performs a logical XOR with the given bit string.
	inline BitStr operator^(const BitStr& string) const {
		return BitStr(m_bits ^ string.m_bits);
	}

	/// Performs a logical OR in-place with the given bit string.
	inline BitStr& operator+=(const BitStr& string) {
		m_bits |= string.m_bits;
		return *this;
	}

	/// Performs a logical AND in-place with the negation of the given bit
	/// string (i.e., sets the bits to `0` that are `1` in the given bit
	/// string).
	inline BitStr& operator-=(const BitStr& string) {
		m_bits &= ~string.m_bits;
		return *this;
	}

	/// Performs a logical AND in-place with the given bit string.
	inline BitStr& operator&=(const BitStr& string) {
		m_bits &= string.m_bits;
		return *this;
	}

	/// Performs a logical XOR in-place with the given bit string.
	inline BitStr& operator^=(const BitStr& string) {
		m_bits ^= string.m_bits;
		return *this;
	}

  private:
	/// Constructs a string `000...010...000` where the `bit`-th bit is `1`.
	inline static bits_t toString(const bit_size_t& bit) {
		return 1 << bit;
	}

	/// Constructs a new bit string in which only the bit at the given index is
	/// `1`.
	explicit BitStr(const bits_t& string) : m_bits(string) {}

	/// The bit string.
	bits_t m_bits;
};
} // namespace detail

/// A \ref BitStr containing 32 bits.
using BitString = detail::BitStr<uint32_t>;

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_CORE_BIT_STRING_H
