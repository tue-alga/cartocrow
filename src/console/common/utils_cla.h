/*
Generic methods for command-line applications.
Copyright (C) 2019  Netherlands eScience Center and [IP's institution]

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-10-2019
*/

/// @file
/// @brief Generic methods for command-line applications.
#ifndef CONSOLE_COMMON_UTILS_CLA_H
#define CONSOLE_COMMON_UTILS_CLA_H

#include <string>
#include <vector>


/**
 * Generate a copyright notice for an command-line application.
 * Note that the notice does not end with a newline character.
 * @param interactive whether the executable accepts user input while running.
 * @return the copyright notice.
 */
std::string copyrightNotice(const bool interactive = false);

/**
 * Generate a basic usage message for an command-line application.
 * @param executable_name the name of the executable.
 * @param description a one-line description of the application's purpose.
 * @param sample_arguments command-line arguments that should be part of the sample usage.
 * @return the usage message.
 */
std::string getUsageMessage
(
  const std::string& executable_name,
  const std::string& description,
  const std::vector<std::string>& sample_arguments = {}
);

/**
 * Initialize the processes related to a basic command-line application.
 * @param argc the number of command-line arguments.
 * @param argv the collection of command-line arguments.
 * @param description a one-line description of the application's purpose.
 * @param sample_arguments any command-line arguments that should be part of the sample usage.
 */
void initApplication
(
  int argc,
  char **argv,
  const std::string& description,
  const std::vector<std::string>& sample_arguments = {}
);

#endif //CONSOLE_COMMON_UTILS_CLA_H
