/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

#include "bit_string.h"


namespace geoviz
{

/**@class BitStr
 * @brief An index-accessible string (or array) of bits.
 * @tparam bits_t the bit string storage type. This must be an integer type.
 * @tparam bit_size_t @parblock the type used to access the bits.
 *
 * Note that the bits are indexed starting at 0.
 * @endparblock
 */

/**@fn static bool BitStr::CheckFit(const bit_size_t& bit);
 * @brief Check whether the bit string is large enough to fit a specific bit.
 * @param bit the bit to check.
 * @return Whether the bit string type would fit the specific bit.
 */

/**@fn static BitStr BitStr::FromBit(const bit_size_t& bit)
 * @brief Construct a new bit string from a single bit.
 * @param bit @parblock the only bit of the string that is set to true.
 *
 * Remember that bits are indexed starting at 0.
 * @endparblock
 * @return the new bit string.
 */

/**@fn static BitStr BitStr::FromString(const bits_t& string)
 * @brief Construct a new bit string from a collection of bits expressed as an integer.
 * @param string the collection of bits that are to be set to true in the bit string.
 * @return the new bit string.
 */

/**@fn BitStr::BitStr()
 * @brief Construct a new empty bit string.
 */

/**@fn bool BitStr::IsEmpty() const
 * @brief Check whether the bit string is empty.
 *
 * The bit string is empty if all bits are false.
 * @return whether the bit string is empty.
 */

/**@fn bool BitStr::Overlaps(const BitStr& string) const
 * @brief Check whether this bit string shares any true bits with another bit string.
 * @param string the other bit string.
 * @return whether there is a bit that is true in both strings.
 */

/**@fn const bits_t& BitStr::Get() const
 * @brief Get the collection of true bits expressed as an integer.
 * @return the bits as an integer.
 */

/**@fn bool BitStr::operator[](const bit_size_t& bit) const
 * @brief Get the value of a single bit.
 * @param bit @parblock the bit to access.
 *
 * Remember that bits are indexed starting at 0.
 * @endparblock
 * @return the value of the bit at the specific index.
 */

/**@fn BitStr BitStr::operator+(const bit_size_t& bit) const
 * @brief Clone the bit string and set a single bit to true.
 * @param bit @parblock the bit to set.
 *
 * Remember that bits are indexed starting at 0.
 * @endparblock
 * @return the new bit string after the operation.
 */

/**@fn BitStr BitStr::operator-(const bit_size_t& bit) const
 * @brief Clone the bit string and set a single bit to false.
 * @param bit @parblock the bit to set.
 *
 * Remember that bits are indexed starting at 0.
 * @endparblock
 * @return the new bit string after the operation.
 */

/**@fn BitStr& BitStr::operator+=(const bit_size_t& bit)
 * @brief Set a single bit to true and return the result.
 * @param bit @parblock the bit to set.
 *
 * Remember that bits are indexed starting at 0.
 * @endparblock
 * @return the bit string after the operation.
 */

/**@fn BitStr& BitStr::operator-=(const bit_size_t& bit)
 * @brief Set a single bit to false and return the result.
 * @param bit @parblock the bit to set.
 *
 * Remember that bits are indexed starting at 0.
 * @endparblock
 * @return the bit string after the operation.
 */

/**@fn BitStr BitStr::operator+(const BitStr& string) const
 * @brief Clone the bit string and set a collection of bits to true.
 * @param string the bits to set.
 * @return the new bit string after the operation.
 */

/**@fn BitStr BitStr::operator-(const BitStr& string) const
 * @brief Clone the bit string and set a collection of bits to false.
 * @param string the bits to set.
 * @return the new bit string after the operation.
 */

// Note that Doxygen (incorrectly) adds a space when copying this documentation (e.g. it changes the method to "operator &").
// This should be fixed by Doxygen 1.8.15, which is not known to ubuntu's package manager.
/**@fn BitStr BitStr::operator &(const BitStr& string) const
 * @brief Clone the bit string and restrict the true bits to another bit string.
 * @param string the bits to restrict to.
 * @return the new bit string after the operation.
 */

/**@fn BitStr BitStr::operator^(const BitStr& string) const
 * @brief Construct the exclusive-or combination of this bit string and another.
 * @param string the other bit string.
 * @return the new bit string after the operation.
 */

/**@fn BitStr& BitStr::operator+=(const BitStr& string)
 * @brief Set a collection of bits to true and return the result.
 * @param string the bits to set.
 * @return the bit string after the operation.
 */

/**@fn BitStr& BitStr::operator-=(const BitStr& string)
 * @brief Set a collection of bits to false and return the result.
 * @param string the bits to set.
 * @return the bit string after the operation.
 */

// Note that Doxygen (incorrectly) adds a space when copying this documentation (e.g. it changes the method to "operator &=").
// This should be fixed by Doxygen 1.8.15, which is not known to ubuntu's package manager.
/**@fn BitStr& BitStr::operator &=(const BitStr& string)
 * @brief Restrict the true bits to a collection of bits and return the result.
 * @param string the bits to restrict to.
 * @return the bit string after the operation.
 */

/**@fn BitStr& BitStr::operator^=(const BitStr& string)
 * @brief Apply the exclusive-or combination of this bit string and another and return the result.
 * @param string the other bit string.
 * @return the bit string after the operation.
 */

} // namespace geoviz
