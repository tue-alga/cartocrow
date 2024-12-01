#include "parse_input.h"

std::string getNextLine(std::istream& str) {
	std::string line;
	std::getline(str,line);
	return line;
}

std::vector<std::string> splitIntoTokens(const std::string& line, char delimiter) {
	std::vector<std::string>   result;
	std::stringstream          lineStream(line);
	std::string                cell;

	while(std::getline(lineStream, cell, delimiter))
	{
		result.push_back(cell);
	}
	// This checks for a trailing comma with no data after it.
	if (!lineStream && cell.empty())
	{
		// If there was a trailing comma then add an empty element.
		result.emplace_back("");
	}
	return result;
}

std::unordered_map<std::string, double> parseRegionData(const std::string& s) {
	std::stringstream ss(s);

	std::unordered_map<std::string, double> result;

	while (ss) {
		auto parts = splitIntoTokens(getNextLine(ss), ' ');
		if (parts.size() <= 1) break;
		if (parts.size() != 2) {
			throw std::runtime_error("Input has incorrect format.");
		}
		result[parts[0]] = stod(parts[1]);
	}

	return result;
}
