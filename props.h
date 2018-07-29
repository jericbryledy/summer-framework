#ifndef _SUMMER_UTIL_PROPS_H
#define _SUMMER_UTIL_PROPS_H

#include "conversions.h"

#include <string>
#include <fstream>
#include <unordered_map>

namespace summer::util {

	class PropertyValue {
	public:
		PropertyValue() = default;

		template<typename T>
		PropertyValue(T&& value) : value(std::forward<T>(value)) {}

		template <typename T>
		operator T() const {
			return convert<T>(value);
		}

		operator std::string() const {
			return value;
		}

	private:
		std::string value;
	};

	class Properties {
	public:
		template <typename T>
		bool load(T&& path) noexcept {
			std::ifstream file(std::forward<T>(path));

			if (file.is_open()) {
				std::string line;

				while (!file.eof()) {
					std::getline(file, line);

					if (auto splitIndex = line.find('='); splitIndex != std::string::npos) {
						auto key = line.substr(0, splitIndex);
						auto value = line.substr(splitIndex + 1);

						values[std::move(key)] = std::move(value);
					}
				}

				return true;
			}

			return false;
		}

		template <typename T>
		const PropertyValue& operator[] (T&& key) noexcept {
			return values[std::forward<T>(key)];
		}

	public:
		std::unordered_map<std::string, PropertyValue> values;
	};

}

#endif
