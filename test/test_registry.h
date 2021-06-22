/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 08-10-2020
*/

#ifndef GEOVIZ_TEST_TEST_REGISTRY_H
#define GEOVIZ_TEST_TEST_REGISTRY_H

#include <array>
#include <map>

#include <glog/logging.h>


template <typename T_, size_t P_, size_t V_> class DeallocNone;

using ptr_t = void*;

/**@brief A container for thread-safe memory blocks.
 *
 * Several processes can register to claim a block of values of one specific type and access them based on a pointer to their own object.
 *
 * On destruction of this container a predetermined functor is fed the memory blocks.
 * @tparam T_ the type of values stored in the blocks.
 * @tparam P_ the number of allowed processes.
 * @tparam V_ the number of allowed values per process.
 * @tparam Dealloc_ the functor to run on destruction.
 */
template <typename T_, size_t P_, size_t V_, typename Dealloc_ = DeallocNone<T_, P_, V_> >
class Registry
{
  using KeyMap = std::map<ptr_t, size_t>;
  using Key = KeyMap::value_type;

 public:
  using Block = std::pair<std::string, std::array<T_, V_> >;
  using Memory = std::array<Block, P_>;

  /**@brief Create a new registry.
   */
  Registry() {}

  /**@brief Destroy the registry after passing the memory to the deallocation functor.
   */
  ~Registry()
  {
    try { Dealloc_()(memory_); } catch (...) {}
  }

  /**@brief Register a new process.
   * Note that there is a predefined number of allowed processes P_.
   * @param process the pointer to the process object. This must be a living, non-null object.
   * @param name the name of the process for bookkeeping purposes. This cannot be an empty string.
   */
  inline void Register(const ptr_t process, const std::string& name)
  {
    CHECK_NOTNULL(process);
    CHECK_NE(name, "");
    std::pair<KeyMap::iterator, bool> result = keys_.insert(Key(process, keys_.size())); // TODO(tvl) make atomic.
    CHECK(result.second);
    CHECK_LE(keys_.size(), P_);
    memory_[result.first->second].first = name;
  }

  /**@brief Access a single value.
   * @param process the pointer to the process object. This must be a process that was previously registered.
   * @param value the number of the value to access.
   * @return a reference to the specific value.
   */
  T_& operator()(const ptr_t process, const size_t value)
  {
    CHECK_LE(value, V_);
    KeyMap::const_iterator key = keys_.find(process);
    CHECK(key != keys_.end());
    return memory_[key->second].second[value];
  }

 private:
  KeyMap keys_;
  Memory memory_;
}; // class Registry


/**@brief A registry deallocator that does nothing with the memory.
 * @tparam T_ the type of values stored in the blocks.
 * @tparam P_ the number of allowed processes.
 * @tparam V_ the number of allowed values per process.
 */
template <typename T_, size_t P_, size_t V_>
struct DeallocNone
{
  using Self = DeallocNone<T_, P_, V_>;
  using Memory = typename Registry<T_, P_, V_, Self>::Memory;

  void operator()(const Memory&) const {}
}; // class DeallocNone


#endif //GEOVIZ_TEST_TEST_REGISTRY_H
