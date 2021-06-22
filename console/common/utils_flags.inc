/*
The GeoViz console applications implement algorithmic geo-visualization
methods, developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-02-2020
*/

#ifndef CONSOLE_COMMON_UTILS_FLAGS_INC
#define CONSOLE_COMMON_UTILS_FLAGS_INC

namespace validate
{

template<bool (*F1_)(const std::string&), bool (*F2_)(const std::string&)>
inline bool Or(const std::string& value)
{
  return F1_(value) || F2_(value);
}

template<bool (*F1_)(const std::string&), bool (*F2_)(const std::string&)>
inline bool And(const std::string& value)
{
  return F1_(value) && F2_(value);
}

template<bool (*F_)(const std::string&)>
inline bool Not(const std::string& value)
{
  return !F_(value);
}


#ifndef DOXYGEN_RUNNING

template<typename V_>
class IsBounded<V_, BoundSide::kLower, Closure::kClosed>
{
 public:
  IsBounded(const V_& bound) : bound_(bound) {}
  
  inline bool operator()(const V_& value) const { return bound_ <= value; }
 private:
  V_ bound_;
}; // class IsBounded<V_, BoundSide::kLower, Closure::kClosed>

template<typename V_>
class IsBounded<V_, BoundSide::kLower, Closure::kOpen>
{
 public:
  IsBounded(const V_& bound) : bound_(bound) {}

  inline bool operator()(const V_& value) const { return bound_ < value; }

 private:
  V_ bound_;
}; // class IsBounded<V_, BoundSide::kLower, Closure::kOpen>

template<typename V_>
class IsBounded<V_, BoundSide::kUpper, Closure::kClosed>
{
 public:
  IsBounded(const V_& bound) : bound_(bound) {}

  inline bool operator()(const V_& value) const { return value <= bound_; }

 private:
  V_ bound_;
}; // class IsBounded<V_, BoundSide::kUpper, Closure::kClosed>

template<typename V_>
class IsBounded<V_, BoundSide::kUpper, Closure::kOpen>
{
 public:
  IsBounded(const V_& bound) : bound_(bound) {}

  inline bool operator()(const V_& value) const { return value < bound_; }

 private:
  V_ bound_;
}; // class IsBounded<V_, BoundSide::kUpper, Closure::kOpen>

template <typename V_, Closure closed_lower /*= Closure::kClosed*/, Closure closed_upper /*= closed_lower*/>
class InRange
{
 public:
  InRange(const V_& lower, const V_& upper) : lower_bound_(lower), upper_bound_(upper) {}

  inline bool operator()(const V_& value) const { return lower_bound_(value) && upper_bound_(value); }

 private:
  IsBounded<V_, BoundSide::kLower, closed_lower> lower_bound_;
  IsBounded<V_, BoundSide::kUpper, closed_upper> upper_bound_;
}; // class InRange


template <typename V_>
class Equals
{
 public:
  Equals(const V_& target) : target_(target) {}

  inline bool operator()(const V_& value) const { return value = target_; }
 private:
  V_ target_;
}; // class Equals


template <BoundSide side, typename V_>
IsBounded<V_, side> MakeBoundedCheck(const V_& bound)
{
  return IsBounded<V_, side>(bound);
}

template <BoundSide side, Closure closed, typename V_>
IsBounded<V_, side, closed> MakeBoundedCheck(const V_& bound)
{
  return IsBounded<V_, side, closed>(bound);
}

template <typename V_>
IsBounded<V_, BoundSide::kUpper> MakeUpperBoundCheck(const V_& bound)
{
  return IsBounded<V_, BoundSide::kUpper>(bound);
}

template <typename V_>
IsBounded<V_, BoundSide::kUpper, Closure::kOpen> MakeStrictUpperBoundCheck(const V_& bound)
{
  return IsBounded<V_, BoundSide::kUpper, Closure::kOpen>(bound);
}

template <typename V_>
IsBounded<V_, BoundSide::kLower> MakeLowerBoundCheck(const V_& bound)
{
  return IsBounded<V_, BoundSide::kLower>(bound);
}

template <typename V_>
IsBounded<V_, BoundSide::kLower, Closure::kOpen> MakeStrictLowerBoundCheck(const V_& bound)
{
  return IsBounded<V_, BoundSide::kLower, Closure::kOpen>(bound);
}


template <Closure closed_lower, Closure closed_upper, typename V_>
InRange<V_, closed_lower, closed_upper> MakeRangeCheck(const V_& lower, const V_& upper)
{
  return InRange<V_, closed_lower, closed_upper>(lower, upper);
}

template <Closure closed, typename V_>
InRange<V_, closed> MakeRangeCheck(const V_& lower, const V_& upper)
{
  return InRange<V_, closed>(lower, upper);
}

template <typename V_>
InRange<V_> MakeRangeCheck(const V_& lower, const V_& upper)
{
  return InRange<V_>(lower, upper);
}

template <typename V_>
Equals<V_> MakeEqualsCheck(const V_& bound)
{
  return Equals<V_>(bound);
}

template <typename V_>
IsBounded<V_, BoundSide::kLower, Closure::kOpen> IsPositive()
{
  return IsBounded<V_, BoundSide::kLower, Closure::kOpen>(0);
}

template <typename V_>
IsBounded<V_, BoundSide::kLower, Closure::kClosed> IsStrictlyPositive()
{
  return IsBounded<V_, BoundSide::kLower, Closure::kClosed>(0);
}

template <typename V_>
IsBounded<V_, BoundSide::kUpper, Closure::kOpen> IsNegative()
{
  return IsBounded<V_, BoundSide::kUpper, Closure::kOpen>(0);
}

template <typename V_>
IsBounded<V_, BoundSide::kUpper, Closure::kClosed> IsStrictlyNegative()
{
  IsBounded<V_, BoundSide::kUpper, Closure::kClosed>(0);
}

#endif // DOXYGEN_RUNNING

} // namespace validate

#endif //CONSOLE_COMMON_UTILS_FLAGS_INC
