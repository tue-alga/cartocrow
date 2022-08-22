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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-11-2020
*/

#ifndef CARTOCROW_CORE_CIRCULATOR_H
#define CARTOCROW_CORE_CIRCULATOR_H

#include <iterator>
#include <type_traits>

namespace cartocrow {
namespace detail {

/// The base class for circular iterators. A circular iterator has all the
/// basic functionality expected of an iterator. Additionally, when the iterator
/// would be incremented past the last element in the container, it is moved to
/// the first element in the container instead. When the iterator would be
/// decremented past the first element in the container, it is moved to the last
/// element in the container instead.
///
/// \todo Why are we not using CGAL's circulators instead?
///
/// \tparam C_ The type of container to iterate over.
/// \tparam I_ The base type of (non-circular) iterator to adapt.
/// \tparam Self The type of the circulator itself.
template <typename C_, typename I_, typename Self> struct CirculatorBase {
	using Container = C_;
	using Iterator = I_;

	using value_type = typename Iterator::value_type;
	using reference = typename Iterator::reference;
	using pointer = typename Iterator::pointer;
	using iterator_category = typename Iterator::iterator_category;
	using difference_type = typename Iterator::difference_type;

	/// Constructs a default circulator, not linked to any container.
	CirculatorBase() : cursor_(), container_() {}

	/// Constructs a copy of an existing circulator.
	CirculatorBase(const CirculatorBase& circulator)
	    : cursor_(circulator.cursor_), container_(circulator.container_) {}

	/// Constructs a circulator pointing to the given iterator of the given
	/// container.
	CirculatorBase(const Iterator& iterator, Container& container)
	    : cursor_(iterator), container_(container) {
		if (cursor_ == container_.end()) {
			cursor_ = container_.begin();
		}
	}

	/// Constructs a circulator pointing to the first element of the given
	/// container.
	explicit CirculatorBase(Container& container) : CirculatorBase(container.begin(), container) {}

	/// Returns a reference to the value pointed to by this circulator.
	inline reference operator*() const {
		return *cursor_;
	}

	/// Returns a pointer to the value pointed to by this circulator.
	inline pointer operator->() const {
		return &*cursor_;
	}

	/// Casts the circulator to an iterator.
	operator Iterator&() {
		return cursor_;
	}

	/// Casts the circulator to a const iterator.
	operator const Iterator&() const {
		return cursor_;
	}

	/// Pre-increments the circulator.
	Self& operator++() {
		++cursor_;
		if (cursor_ == container_.end()) {
			cursor_ = container_.begin();
		}
		return static_cast<Self&>(*this);
	}

	/// Post-increments the circulator.
	Self operator++(int) {
		Self tmp = *static_cast<Self*>(this);
		++(*this);
		return tmp;
	}

	/// Pre-decrements the circulator.
	Self& operator--() {
		if (cursor_ == container_.begin()) {
			cursor_ = container_.end();
		}
		--cursor_;
		return static_cast<Self&>(*this);
	}

	/// Post-decrements the circulator.
	Self operator--(int) {
		Self tmp = *static_cast<Self*>(this);
		--(*this);
		return tmp;
	}

	/// Checks if this circulator points to the same element as another
	/// circulator.
	inline bool operator==(const CirculatorBase& other) const {
		return cursor_ == other.cursor_;
	}

	/// Checks if this circulator points to a different element than
	/// another circulator.
	inline bool operator!=(const CirculatorBase& other) const {
		return cursor_ != other.cursor_;
	}

  private:
	I_ cursor_;
	C_& container_;
};

} // namespace detail

/// A circular iterator.
///
/// A circular iterator has all the basic functionality expected of an iterator.
/// Additionally, when the iterator would be incremented past the last element
/// in the container, it is moved to the first element in the container instead.
/// When the iterator would be decremented past the first element in the
/// container, it is moved to the last element in the container instead.
///
/// \tparam C_ The type of container to iterate over.
/// \tparam I_ The base type of (non-circular) iterator to adapt.
template <typename C_, typename I_ = typename C_::iterator>
struct Circulator : public detail::CirculatorBase<C_, I_, Circulator<C_, I_>> {
	using Base = detail::CirculatorBase<C_, I_, Circulator<C_, I_>>;

	using Container = typename Base::Container;
	using Iterator = typename Base::Iterator;

	using value_type = typename Base::value_type;
	using reference = typename Base::reference;
	using pointer = typename Base::pointer;
	using iterator_category = typename Base::iterator_category;
	using difference_type = typename Base::difference_type;

	/// Constructs a default circulator, not linked to any container.
	Circulator() : Base() {}

	/// Constructs a copy of an existing circulator.
	Circulator(const Circulator& circulator) : Base(circulator) {}

	/// Constructs a circulator pointing to the given iterator of the given
	/// container.
	Circulator(const Iterator& iterator, Container& container) : Base(iterator, container) {}

	/// Constructs a circulator pointing to the first element of the given
	/// container.
	explicit Circulator(Container& container) : Base(container) {}
};

/// A const circular iterator.
///
/// A circular iterator has all the basic functionality expected of an iterator.
/// Additionally, when the iterator would be incremented past the last element
/// in the container, it is moved to the first element in the container instead.
/// When the iterator would be decremented past the first element in the
/// container, it is moved to the last element in the container instead.
///
/// \tparam C_ The type of container to iterate over.
/// \tparam I_ The base type of (non-circular) iterator to adapt.
template <typename C_, typename I_ = typename C_::const_iterator>
struct ConstCirculator
    : public detail::CirculatorBase<typename std::add_const<C_>::type, I_,
                                    Circulator<typename std::add_const<C_>::type, I_>> {
	using Base = detail::CirculatorBase<typename std::add_const<C_>::type, I_,
	                                    Circulator<typename std::add_const<C_>::type, I_>>;

	using Container = typename Base::Container;
	using Iterator = typename Base::Iterator;

	using value_type = typename Base::value_type;
	using reference = typename Base::reference;
	using pointer = typename Base::pointer;
	using iterator_category = typename Base::iterator_category;
	using difference_type = typename Base::difference_type;

	/// Constructs a default circulator, not linked to any container.
	ConstCirculator() : Base() {}

	/// Constructs a copy of an existing circulator.
	ConstCirculator(const ConstCirculator& circulator) : Base(circulator) {}

	/// Constructs a circulator pointing to the given iterator of the given
	/// container.
	ConstCirculator(const Iterator& iterator, const Container& container)
	    : Base(iterator, container) {}

	/// Constructs a circulator pointing to the first element of the given
	/// container.
	explicit ConstCirculator(const Container& container) : Base(container) {}
};

template <typename Container, typename Iterator>
ConstCirculator<Container, Iterator> make_circulator(const Iterator& iterator,
                                                     const Container& container) {
	return ConstCirculator<Container, Iterator>(iterator, container);
}

template <typename Container>
ConstCirculator<Container> make_circulator(const Container& container) {
	return ConstCirculator<Container>(container);
}

template <typename Container, typename Iterator>
Circulator<Container, Iterator> make_circulator(const Iterator& iterator, Container& container) {
	return Circulator<Container, Iterator>(iterator, container);
}

template <typename Container> Circulator<Container> make_circulator(Container& container) {
	return Circulator<Container>(container);
}

} // namespace cartocrow

#endif //CARTOCROW_CORE_CIRCULATOR_H
