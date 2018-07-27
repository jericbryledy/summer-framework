#ifndef _SUMMER_H
#define _SUMMER_H

#include <type_traits>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <list>

namespace summer {

	class SingletonBase {
	public:
		virtual ~SingletonBase() noexcept {}
	};

	template <typename T>
	class SingletonIdentifier {
	public:
		SingletonIdentifier() noexcept : SingletonIdentifier(typeid(T).name()) {}

		SingletonIdentifier(const char* name) noexcept : name(name) {}
		SingletonIdentifier(const std::string& name) noexcept : name(name) {}
		SingletonIdentifier(const std::string&& name) noexcept : name(std::move(name)) {}

		const std::string& getName() const noexcept {
			return name;
		}

	public:
		using type = T;

	private:
		std::string name;
	};

	class ApplicationContext {
	public:
		template <typename T>
		T* getSingleton(const SingletonIdentifier<T>& singIden) noexcept {
			return static_cast<T*>(singletons[singIden.getName()].get());
		}

		template <typename T, typename ... Params>
		void registerSingleton(const SingletonIdentifier<T>& singIden, const Params& ... params) noexcept {
			if (singletonInstancers.find(singIden.getName()) != std::end(singletonInstancers)) {
				std::cerr << "singleton [" << singIden.getName() << "] already registered" << std::endl;

				return;
			}

			std::function<bool()> instantiator = [=]() {
				if (!prerequisitesReady(params...)) {
					return false;
				}

				singletons[singIden.getName()] = std::make_unique<T>(getParam(params)...);

				return true;
			};

			std::function<void()> errorLogger = [=]() {
				std::cerr << "error instantiating [" << singIden.getName() << ']' << std::endl;
				printMissing(1, params...);
			};

			singletonInstancers[singIden.getName()] = std::make_pair(instantiator, errorLogger);
		}

		bool instantiateSingletons() noexcept {
			for (auto progress = true; progress; ) {
				progress = false;

				for (auto iter = std::begin(singletonInstancers); iter != std::end(singletonInstancers); ) {
					if (std::get<0>(iter->second)()) {
						iter = singletonInstancers.erase(iter);
						progress = true;
					} else {
						++iter;
					}
				}
			}

			for (auto iter = std::begin(singletonInstancers); iter != std::end(singletonInstancers); ++iter) {
				std::function<void()> errorLogger;
				std::tie(std::ignore, errorLogger) = iter->second;

				errorLogger();
			}

			return !singletonInstancers.size();
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<SingletonBase>> singletons;
		std::unordered_map<std::string, std::tuple<std::function<bool()>, std::function<void()>>> singletonInstancers;

		template <typename T>
		T& getParam(T& param) noexcept {
			return param;
		}

		template <typename T>
		T& getParam(const SingletonIdentifier<T>& singIden) noexcept {
			return *getSingleton(singIden);
		}

		bool prerequisitesReady() noexcept { return true; }

		template <typename T, typename ... Params>
		bool prerequisitesReady(const T& param, const Params& ... params) noexcept {
			return prerequisitesReady(params...);
		}

		template <typename T, typename ... Params>
		bool prerequisitesReady(const SingletonIdentifier<T>& singIden, const Params& ... params) noexcept {
			if (getSingleton(singIden) != nullptr) {
				return prerequisitesReady(params...);
			}

			return false;
		}

		void printMissing(int) noexcept {}

		template <typename T, typename ... Params>
		void printMissing(int index, const T& param, const Params& ... params) noexcept {
			printMissing(index + 1, params...);
		}

		template <typename T, typename ... Params>
		void printMissing(int index, const SingletonIdentifier<T>& singIden, const Params& ... params) noexcept {
			if (getSingleton(singIden) == nullptr) {
				std::cerr << "  - param " << index << " [" << singIden.getName() << "] missing" << std::endl;
			}

			printMissing(index + 1, params...);
		}
	};

	class PrimarySource {
	public:
		void setup(const ApplicationContext& context) noexcept {
			std::cout << "default setup" << std::endl;
		}
	};

	enum class Method {
		POST, GET, PUT, PATCH, DELETE
	};

	class HttpRequest {
	public:
		Method getMethod() const noexcept {
			return method;
		}

		std::string getPath() const noexcept {
			return path;
		}

	private:
		Method method;
		std::string path;
	};

	class HttpResponse {

	};

	class Controller {
	private:
		using handlerType = std::function<HttpResponse(HttpRequest &request)>;

	public:
		void registerHandler(Method method, handlerType func) noexcept {
			handlers.emplace_back(func);
		}

	private:
		std::vector<handlerType> handlers;
	};

	template <typename T, typename std::enable_if_t<std::is_base_of_v<PrimarySource, T>>* = nullptr>
	class SummerApplication {
	public:

	private:
		void initialize(int argc, char **argv) noexcept {
			summerApp.setup(context);
			context.instantiateSingletons();
		}

		ApplicationContext context;
		T summerApp;

	public:
		static void run(int argc, char **argv) noexcept {
			SummerApplication<T> summerApp;

			summerApp.initialize(argc, argv);
		}

	};

}

#endif
