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

#ifndef CONSOLE_COMMON_UTILS_FLAGS_H
#define CONSOLE_COMMON_UTILS_FLAGS_H

#include <functional>

#include <gflags/gflags.h>
#include <glog/logging.h>


namespace validate
{

#ifndef DOXYGEN_RUNNING

bool IsFile(const std::string& value);
bool IsDirectory(const std::string& value);

bool ExistsFile(const std::string& value);
bool ExistsDirectory(const std::string& value);
bool ExistsPath(const std::string& value);

bool AvailableFile(const std::string& value);  // Does not exist, but parent directory does.
bool MakeAvailableFile(const std::string& value);  // Make sure parent directory exists.

bool Empty(const std::string& value);

template<bool (*F1_)(const std::string&), bool (*F2_)(const std::string&)>
bool Or(const std::string& value);
template<bool (*F1_)(const std::string&), bool (*F2_)(const std::string&)>
bool And(const std::string& value);
template<bool (*F_)(const std::string&)>
bool Not(const std::string& value);



enum BoundSide { kLower, kUpper };
enum Closure { kClosed, kOpen };

template <typename V_, BoundSide side, Closure closed = Closure::kClosed>
class IsBounded;

template <typename V_, Closure closed_lower = Closure::kClosed, Closure closed_upper = closed_lower>
class InRange;

template <typename V_>
class Equals;


// Convenience methods.

template <BoundSide side, typename V_>
IsBounded<V_, side> MakeBoundedCheck(const V_& bound);

template <BoundSide side, Closure closed, typename V_>
IsBounded<V_, side, closed> MakeBoundedCheck(const V_& bound);

template <typename V_>
IsBounded<V_, BoundSide::kUpper> MakeUpperBoundCheck(const V_& bound);

template <typename V_>
IsBounded<V_, BoundSide::kUpper, Closure::kOpen> MakeStrictUpperBoundCheck(const V_& bound);

template <typename V_>
IsBounded<V_, BoundSide::kLower> MakeLowerBoundCheck(const V_& bound);

template <typename V_>
IsBounded<V_, BoundSide::kLower, Closure::kOpen> MakeStrictLowerBoundCheck(const V_& bound);


template <Closure closed_lower, Closure closed_upper, typename V_>
InRange<V_, closed_lower, closed_upper> MakeRangeCheck(const V_& lower, const V_& upper);

template <Closure closed, typename V_>
InRange<V_, closed> MakeRangeCheck(const V_& lower, const V_& upper);

template <typename V_>
InRange<V_> MakeRangeCheck(const V_& lower, const V_& upper);

template <typename V_>
Equals<V_> MakeEqualsCheck(const V_& bound);


template <typename V_>
IsBounded<V_, BoundSide::kLower, Closure::kOpen> IsPositive();

template <typename V_>
IsBounded<V_, BoundSide::kLower, Closure::kClosed> IsStrictlyPositive();

template <typename V_>
IsBounded<V_, BoundSide::kUpper, Closure::kOpen> IsNegative();

template <typename V_>
IsBounded<V_, BoundSide::kUpper, Closure::kClosed> IsStrictlyNegative();

#endif // DOXYGEN_RUNNING

} // namespace validate


#define FLAGS_NAME_AND_VALUE(v) #v, FLAGS_ ## v

// Log a flag's name and value.
template<typename F_>
inline void PrintFlag(const std::string& name, const F_& flag)
{
  LOG(INFO) << "  " << name << ":\t\"" << flag << "\"";
}

// Log a flag's name and value.
template<>
inline void PrintFlag<bool>(const std::string& name, const bool& flag)
{
  LOG(INFO) << "  " << name << ":\t\"" << (flag ? "TRUE" : "FALSE") << "\"";
}

// Log a flag and apply a boolean function to validate it.
template<typename V_>
inline bool CheckAndPrintFlag(const std::string& name, const V_& value, std::function<bool(const V_&)> check)
{
  PrintFlag(name, value);
  const bool correct = check(value);
  LOG_IF(INFO, !correct) << "  --- ERROR! ---";
  return correct;
}

// Log a flag and apply a boolean operator to validate it.
template<typename V_>
inline bool CheckAndPrintFlag(const std::string& name, const V_& value, bool(*check)(const V_&))
{
  return CheckAndPrintFlag(name, value, std::function<bool(const V_&)>(check));
}

// Log a flag and apply a boolean functor to validate it.
template<typename V_, typename F_>
inline bool CheckAndPrintFlag(const std::string& name, const V_& value, F_ check)
{
  return CheckAndPrintFlag(name, value, std::function<bool(const V_&)>(check));
}

// Check whether exactly one of the flags is not empty.
inline bool CheckMutuallyExclusive
(
  const std::string& name_1,
  const std::string& value_1,
  const std::string& name_2,
  const std::string& value_2
)
{
  const bool correct = value_1.empty() || value_2.empty();
  LOG_IF(INFO, !correct) << "  --- ERROR: " << name_1 << " XOR " << name_2 << " ---";
  return correct;
}

#include "utils_flags.inc"

#endif //CONSOLE_COMMON_UTILS_FLAGS_H
