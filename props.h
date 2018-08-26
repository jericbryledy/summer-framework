#ifndef _SUMMER_UTIL_PROPS_H
#define _SUMMER_UTIL_PROPS_H

#include "conversions.h"

#include <string>
#include <fstream>
#include <unordered_map>

namespace summer::util {

	class property_value {
	public:
		property_value() = default;

		template<typename T>
		property_value(T&& value) : value_(std::forward<T>(value)) {}

		template <typename T>
		operator T() const {
			return convert<T>(value_);
		}

		operator std::string() const {
			return value_;
		}

	private:
		std::string value_;
	};

	class properties {
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

					if (auto split_index = line.find('='); split_index != std::string::npos) {
						auto key = line.substr(0, split_index);
						auto value = line.substr(split_index + 1);

						values_.emplace(std::move(key), std::move(value));
					}
				}

				return true;
			}

			return false;
		}

		template <typename T>
		const property_value& operator[] (T&& key) const noexcept {
			return get_property(std::forward<T>(key));
		}

		template <typename T>
		const property_value& get_property(T&& key) const noexcept {
			if (auto res = values_.find(std::forward<T>(key)); res != values_.end()) {
				return res->second;
			}

			return {};
		}

		template <typename T, typename U, typename V = std::conditional_t<std::is_convertible_v<U, std::string>, std::string, U>>
		V get_property(T&& key, U default_value) const noexcept {
			if (auto res = values_.find(std::forward<T>(key)); res != values_.end()) {
				return res->second;
			}

			return default_value;
		}


	private:
		std::unordered_map<std::string, property_value> values_;
	};

}

#endif
