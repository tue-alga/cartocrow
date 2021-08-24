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

#ifndef CARTOCROW_COMMON_CIRCULATOR_H
#define CARTOCROW_COMMON_CIRCULATOR_H

#include <iterator>
#include <type_traits>

namespace cartocrow {
namespace detail {

template <typename C_, typename I_, typename Self> struct CirculatorBase {
	using Container = C_;
	using Iterator = I_;

	using value_type = typename Iterator::value_type;
	using reference = typename Iterator::reference;
	using pointer = typename Iterator::pointer;
	using iterator_category = typename Iterator::iterator_category;
	using difference_type = typename Iterator::difference_type;

	CirculatorBase() : cursor_(), container_() {}

	CirculatorBase(const CirculatorBase& circulator)
	    : cursor_(circulator.cursor_), container_(circulator.container_) {}

	CirculatorBase(const Iterator& iterator, Container& container)
	    : cursor_(iterator), container_(container) {
		if (cursor_ == container_.end())
			cursor_ = container_.begin();
	}

	explicit CirculatorBase(Container& container) : CirculatorBase(container.begin(), container) {}

	inline reference operator*() const {
		return *cursor_;
	}

	inline pointer operator->() const {
		return &*cursor_;
	}

	operator Iterator&() {
		return cursor_;
	}

	operator const Iterator&() const {
		return cursor_;
	}

	Self& operator++() {
		++cursor_;
		if (cursor_ == container_.end())
			cursor_ = container_.begin();
		return static_cast<Self&>(*this);
	}

	Self operator++(int) {
		Self tmp = *static_cast<Self*>(this);
		++(*this);
		return tmp;
	}

	Self& operator--() {
		if (cursor_ == container_.begin())
			cursor_ = container_.end();
		--cursor_;
		return static_cast<Self&>(*this);
	}

	Self operator--(int) {
		Self tmp = *static_cast<Self*>(this);
		--(*this);
		return tmp;
	}

	inline bool operator==(const CirculatorBase& other) const {
		return cursor_ == other.cursor_;
	}

	inline bool operator!=(const CirculatorBase& other) const {
		return cursor_ != other.cursor_;
	}

  private:
	I_ cursor_;
	C_& container_;
}; // class CirculatorBase

} // namespace detail

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

	Circulator() : Base() {}

	Circulator(const Circulator& circulator) : Base(circulator) {}

	Circulator(const Iterator& iterator, Container& container) : Base(iterator, container) {}

	explicit Circulator(Container& container) : Base(container) {}
}; // class Circulator

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

	ConstCirculator() : Base() {}

	ConstCirculator(const ConstCirculator& circulator) : Base(circulator) {}

	ConstCirculator(const Iterator& iterator, const Container& container)
	    : Base(iterator, container) {}

	explicit ConstCirculator(const Container& container) : Base(container) {}
}; // class ConstCirculator

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

#endif //CARTOCROW_COMMON_CIRCULATOR_H
