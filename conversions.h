#ifndef _SUMMER_UTIL_CONVERSIONS_H
#define _SUMMER_UTIL_CONVERSIONS_H

#include <string>

namespace summer::util {
	
	template <typename T>
	T convert(std::string const& input);

	template <>
	int convert(std::string const& input) {
		return std::stoi(input);
	}

	template <>
	long convert(std::string const& input) {
		return std::stol(input);
	}

	template <>
	unsigned long convert(std::string const& input) {
		return std::stoul(input);
	}

	template <>
	long long convert(std::string const& input) {
		return std::stoll(input);
	}

	template <>
	unsigned long long convert(std::string const& input) {
		return std::stoull(input);
	}

	template <>
	float convert(std::string const& input) {
		return std::stof(input);
	}

	template <>
	double convert(std::string const& input) {
		return std::stod(input);
	}

	template <>
	long double convert(std::string const& input) {
		return std::stold(input);
	}

}

#endif
