/*
Copyright (C) 2026  TU Eindhoven

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
*/                                                                                                \

#pragma once

#include <nlohmann/json.hpp>
#include "../core/core.h"

namespace cartocrow::utils {

class PersistentSettings {
  private:
	using Key = std::string;
	using Json = nlohmann::json;

  public:
	PersistentSettings(std::string settings = "settings");
	~PersistentSettings();

	void setColor(Key key, Color value);
	Color getColor(Key key, Color default_value);

	void setPath(Key key, std::filesystem::path value);
	std::filesystem::path getPath(Key key, std::filesystem::path default_value);

	void setString(Key key, std::string value);
	std::string getString(Key key, std::string default_value);

	void setBoolean(Key key, bool value);
	bool getBoolean(Key key, bool default_value);

	void setInteger(Key key, int value);
	int getInteger(Key key, int default_value);

	void setDouble(Key key, double value);
	int getDouble(Key key, double default_value);

  private:
	std::string m_path;
	Json m_json;
	bool m_changed;
};

} // namespace cartocrow::utils