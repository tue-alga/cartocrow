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
*/

#include "persistent_settings.h"

#include <fstream>

namespace cartocrow::utils {

PersistentSettings::PersistentSettings(std::string settings) {
	m_path = settings + ".json";
	std::ifstream f(m_path);
	if (!f.fail()) {
		f >> m_json;
	}
	m_changed = false;
}

PersistentSettings::~PersistentSettings() {
	if (m_changed) {
		std::ofstream f(m_path);
		f << std::setw(4) << m_json << std::endl;
	}
}

void PersistentSettings::setColor(Key key, Color value) {
	m_json[key] = {value.r, value.g, value.b};
	m_changed = true;
}

Color PersistentSettings::getColor(Key key, Color default_value) {
	Json& obj = m_json[key];
	if (obj.is_null()) {
		return default_value;
	} else {
		std::vector<int> rgb = obj.get<std::vector<int>>();
		return Color{rgb[0], rgb[1], rgb[2]};
	}
}

void PersistentSettings::setPath(Key key, std::filesystem::path value) {
	m_json[key] = value.string();
	m_changed = true;
}

std::filesystem::path PersistentSettings::getPath(Key key, std::filesystem::path default_value) {
	Json& obj = m_json[key];
	if (obj.is_null()) {
		return default_value;
	} else {
		return obj.get<std::string>();
	}
}

void PersistentSettings::setString(Key key, std::string value) {
	m_json[key] = value;
	m_changed = true;
}

std::string PersistentSettings::getString(Key key, std::string default_value) {
	Json& obj = m_json[key];
	if (obj.is_null()) {
		return default_value;
	} else {
		return obj.get<std::string>();
	}
}

void PersistentSettings::setBoolean(Key key, bool value) {
	m_json[key] = value;
	m_changed = true;
}

bool PersistentSettings::getBoolean(Key key, bool default_value) {
	Json& obj = m_json[key];
	if (obj.is_null()) {
		return default_value;
	} else {
		return obj.get<bool>();
	}
}

void PersistentSettings::setInteger(Key key, int value) {
	m_json[key] = value;
	m_changed = true;
}

int PersistentSettings::getInteger(Key key, int default_value) {
	Json& obj = m_json[key];
	if (obj.is_null()) {
		return default_value;
	} else {
		return obj.get<int>();
	}
}

void PersistentSettings::setDouble(Key key, double value) {
	m_json[key] = value;
	m_changed = true;
}

int PersistentSettings::getDouble(Key key, double default_value) {
	Json& obj = m_json[key];
	if (obj.is_null()) {
		return default_value;
	} else {
		return obj.get<double>();
	}
}

} // namespace cartocrow::utils
