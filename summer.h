#ifndef _SUMMER_H
#define _SUMMER_H

#include <type_traits>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>

namespace summer {

	class SingletonBase {
	public:
		virtual ~SingletonBase() {}
	};

	template <typename T>
	struct SingletonIdentifier {
		std::string name;
		using type = T;
	};

	class ApplicationContext {
	public:
		template<typename T>
		T* getSingleton(const std::string &name) noexcept {
			return static_cast<T*>(singletons[name].get());
		}

		template<typename T, typename ... SingIdens>
		T* registerSingleton(const std::string &name, SingIdens ... singIdens) noexcept {
			if (prerequisitesReady(singIdens...)) {
				std::cout << "ready!" << std::endl;
			} else {
				std::cout << "not ready!" << std::endl;
			}

			singletons[name] = std::make_unique<T>(getSingleton<typename SingIdens::type>(singIdens.name)...);
			return getSingleton<T>(name);
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<SingletonBase>> singletons;

		bool prerequisitesReady() { return true; }

		template<typename T, typename ... SingIdens>
		bool prerequisitesReady(SingletonIdentifier<T> singIden, SingIdens ... singIdens) {
			auto ready = getSingleton<T>(singIden.name) != nullptr;

			if (ready) {
				return prerequisitesReady(singIdens...);
			}

			return false;
		}
	};

	class PrimarySource {
	public:
		void setup(ApplicationContext& context) noexcept {
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
		void registerHandler(Method method, handlerType func) {
			handlers.emplace_back(func);
		}

	private:
		std::vector<handlerType> handlers;
	};

	template<typename T, typename std::enable_if_t<std::is_base_of_v<PrimarySource, T>>* = nullptr>
	class SummerApplication {
	public:

	private:
		void initialize(int argc, char **argv) noexcept {
			summerApp.setup(context);
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
