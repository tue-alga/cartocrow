/*
The GeoViz library implements algorithmic geo-visualization methods,
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-11-2020
*/

#include "circulator.h"

namespace geoviz
{

/**@struct CirculatorBase
 * @brief The base class for circular iterators.
 * A circular iterator has all the basic functionality expected of an iterator.
 *
 * Additionally, when the iterator would be incremented past the last element in the container, it is moved to the first element in the container instead.
 * When the iterator would be decremented past the first element in the container, it is moved to the last element in the container instead.
 * @tparam C_ the type of container to iterate over.
 * @tparam I_ the base type of (non-circular) iterator to adapt.
 * @tparam Self @parblock the type of this class itself.
 *
 * Note that this class uses the curiously recurring template pattern to be able to have some methods of the base class return types of the derived class.
 * @endparblock
 */

/**@fn CirculatorBase::Container
 * @brief The type of container to circulate.
 */

/**@fn CirculatorBase::Iterator
 * @brief The base type of iterator used to circulate.
 */

/**@fn CirculatorBase::value_type
 * @brief The type of value accessed through the circulator.
 */

/**@fn CirculatorBase::reference
 * @brief The type of reference accessed through the circulator.
 */

/**@fn CirculatorBase::pointer
 * @brief The type of pointer accessed through the circulator.
 */

/**@fn CirculatorBase::pointer
 * @brief The type of pointer accessed through the circulator.
 */

/**@fn CirculatorBase::iterator_category
 * @brief The iterator category of the circulator.
 */

/**@fn CirculatorBase::difference_type
 * @brief The type used to describe the difference bewteen two circulators.
 */

/**@fn CirculatorBase::CirculatorBase()
 * @brief Construct a default circulator.
 * Note that a default circulator is not linked to any container.
 * It cannot be used to access any element.
 */

/**@fn CirculatorBase::CirculatorBase(const CirculatorBase& circulator)
 * @brief Copy construct a circulator.
 * @param circulator the circulator to copy.
 */

/**@fn CirculatorBase::CirculatorBase(const Iterator& iterator, Container& container)
 * @brief Construct a circulator.
 * @param iterator the iterator pointing to the element that the circulator must initially point to.
 * @param container the container to circulate.
 */

/**@fn CirculatorBase::CirculatorBase(Container& container)
 * @brief Construct a circulator pointing to the first element of a container.
 * @param container the container to circulate.
 */

/**@fn CirculatorBase::reference CirculatorBase::operator*() const
 * @brief Access the value currently pointed to.
 * @return the current value.
 */

/**@fn CirculatorBase::pointer CirculatorBase::operator->() const
 * @brief Access the address of the value currently pointed to.
 * @return the address of the current value.
 */

/**@fn CirculatorBase::operator Iterator&()
 * @brief Cast the circulator to an iterator.
 * @return an iterator pointing to the same element as this circulator.
 */

/**@fn CirculatorBase::operator const Iterator&()
 * @brief Cast the circulator to a fixed value iterator.
 * @return a constant value iterator pointing to the same element as this circulator.
 */

/**@fn CirculatorBase::Self& CirculatorBase::operator++()
 * @brief Pre-increment the circulator.
 * @return the circulator after incrementation.
 */

/**@fn CirculatorBase::Self& CirculatorBase::operator++(int)
 * @brief Post-increment the circulator.
 * @return the circulator before incrementation.
 */

/**@fn CirculatorBase::Self& CirculatorBase::operator--()
 * @brief Pre-decrement the circulator.
 * @return the circulator after decrementation.
 */

/**@fn CirculatorBase::Self& CirculatorBase::operator--(int)
 * @brief Post-decrement the circulator.
 * @return the circulator before decrementation.
 */

/**@fn bool CirculatorBase::operator==(const CirculatorBase& other)
 * @brief Check whether this circulator to is the same as another.
 * @param other the circulator to compare to.
 * @return True if and only if this circulator points to the same element as the other circulator.
 */

/**@fn bool CirculatorBase::operator!=(const CirculatorBase& other)
 * @brief Check whether this circulator to is different than another.
 * @param other the circulator to compare to.
 * @return True if and only if this circulator points to a different element as the other circulator.
 */


/**@struct Circulator
 * @brief A circular iterator.
 *
 * This iterator has all the basic functionality expected of an iterator.
 *
 * Additionally, when the iterator would be incremented past the last element in the container, it is moved to the first element in the container instead.
 * When the iterator would be decremented past the first element in the container, it is moved to the last element in the container instead.
 * @tparam Container the type of container to circulate.
 * @tparam Iterator the base type of iterator.
 */

/**@fn Circulator::Container
 * @brief The type of container to circulate.
 */

/**@fn Circulator::Iterator
 * @brief The base type of iterator used to circulate.
 */

/**@fn Circulator::value_type
 * @brief The type of value accessed through the circulator.
 */

/**@fn Circulator::reference
 * @brief The type of reference accessed through the circulator.
 */

/**@fn Circulator::pointer
 * @brief The type of pointer accessed through the circulator.
 */

/**@fn Circulator::pointer
 * @brief The type of pointer accessed through the circulator.
 */

/**@fn Circulator::iterator_category
 * @brief The iterator category of the circulator.
 */

/**@fn Circulator::difference_type
 * @brief The type used to describe the difference bewteen two circulators.
 */

/**@fn Circulator::Circulator(const Circulator& circulator)
 * @brief Copy construct a circulator.
 * @param circulator the circulator to copy.
 */

/**@fn Circulator::Circulator(const Iterator& iterator, Container& container)
 * @brief Construct a circulator.
 * @param iterator the iterator pointing to the element that the circulator must initially point to.
 * @param container the container to circulate.
 */

/**@fn Circulator::Circulator(Container& container)
 * @brief Construct a circulator pointing to the first element of a container.
 * @param container the container to circulate.
 */


/**@struct ConstCirculator
 * @brief A circular constant iterator.
 *
 * This iterator has all the basic functionality expected of a constant iterator.
 *
 * Additionally, when the iterator would be incremented past the last element in the container, it is moved to the first element in the container instead.
 * When the iterator would be decremented past the first element in the container, it is moved to the last element in the container instead.
 * @tparam Container the type of container to circulate.
 * @tparam Iterator the base type of iterator.
 */

/**@fn ConstCirculator::Container
 * @brief The type of container to circulate.
 */

/**@fn ConstCirculator::Iterator
 * @brief The base type of iterator used to circulate.
 */

/**@fn ConstCirculator::value_type
 * @brief The type of value accessed through the circulator.
 */

/**@fn ConstCirculator::reference
 * @brief The type of reference accessed through the circulator.
 */

/**@fn ConstCirculator::pointer
 * @brief The type of pointer accessed through the circulator.
 */

/**@fn ConstCirculator::pointer
 * @brief The type of pointer accessed through the circulator.
 */

/**@fn ConstCirculator::iterator_category
 * @brief The iterator category of the circulator.
 */

/**@fn ConstCirculator::difference_type
 * @brief The type used to describe the difference bewteen two circulators.
 */

/**@fn ConstCirculator::ConstCirculator(const ConstCirculator& circulator)
 * @brief Copy construct a constant circulator.
 * @param circulator the circulator to copy.
 */

/**@fn ConstCirculator::ConstCirculator(const Iterator& iterator, const Container& container)
 * @brief Construct a constant circulator.
 * @param iterator the iterator pointing to the element that the circulator must initially point to.
 * @param container the container to circulate.
 */

/**@fn ConstCirculator::ConstCirculator(const Container& container)
 * @brief Construct a constant circulator pointing to the first element of a container.
 * @param container the container to circulate.
 */

} // namespace geoviz
