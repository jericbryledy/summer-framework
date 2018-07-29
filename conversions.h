#ifndef _SUMMER_UTIL_CONVERSIONS_H
#define _SUMMER_UTIL_CONVERSIONS_H

#include <string>

namespace summer::util {
	
	template <typename T>
	T convert(const std::string& input) {
		static_assert(false, "no default implementation for the specified type");
	}

	template <>
	int convert(const std::string& input) {
		return std::stoi(input);
	}

	template <>
	long convert(const std::string& input) {
		return std::stol(input);
	}

	template <>
	unsigned long convert(const std::string& input) {
		return std::stoul(input);
	}

	template <>
	long long convert(const std::string& input) {
		return std::stoll(input);
	}

	template <>
	unsigned long long convert(const std::string& input) {
		return std::stoull(input);
	}

	template <>
	float convert(const std::string& input) {
		return std::stof(input);
	}

	template <>
	double convert(const std::string& input) {
		return std::stod(input);
	}

	template <>
	long double convert(const std::string& input) {
		return std::stold(input);
	}

}

#endif
