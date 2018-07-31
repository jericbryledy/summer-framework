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

					if (auto c = line[0]; c == '#' || c == '!') {
						continue;
					}

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
		const PropertyValue& operator[] (T&& key) const noexcept {
			return getProperty(std::forward<T>(key));
		}

		template <typename T>
		const PropertyValue& getProperty(T&& key) const noexcept {
			if (auto res = values.find(std::forward<T>(key)); res != values.end()) {
				return res->second;
			}

			return {};
		}

		template <typename T, typename U, typename V = std::conditional_t<std::is_convertible_v<U, std::string>, std::string, U>>
		V getProperty(T&& key, U defaultValue) const noexcept {
			if (auto res = values.find(std::forward<T>(key)); res != values.end()) {
				return res->second;
			}

			return defaultValue;
		}


	private:
		std::unordered_map<std::string, PropertyValue> values;
	};

}

#endif
