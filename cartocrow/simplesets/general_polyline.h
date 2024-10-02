#ifndef CARTOCROW_GENERAL_POLYLINE_H
#define CARTOCROW_GENERAL_POLYLINE_H

#include <vector>

template <class ArrTraits>
class General_polyline_2 {
  public:
	typedef typename std::vector<typename ArrTraits::X_monotone_curve_2>::iterator Curve_iterator;
	typedef typename std::vector<typename ArrTraits::X_monotone_curve_2>::const_iterator Curve_const_iterator;

	template <class InputIterator>
	General_polyline_2(InputIterator begin, InputIterator end) {
		m_xm_curves = std::vector(begin, end);
	}

	Curve_iterator curves_begin() {
		return m_xm_curves.begin();
	}
	Curve_iterator curves_end() {
		return m_xm_curves.end();
	}
	Curve_const_iterator curves_begin() const {
		return m_xm_curves.cbegin();
	}
	Curve_const_iterator curves_end() const {
		return m_xm_curves.cend();
	}
	[[nodiscard]] unsigned int size() const
	{
		return static_cast<unsigned int>(m_xm_curves.size());
	}
  private:
	std::vector<typename ArrTraits::X_monotone_curve_2> m_xm_curves;
};

#endif //CARTOCROW_GENERAL_POLYLINE_H
