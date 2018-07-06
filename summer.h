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
	struct SingletonIdentifier {
		std::string name;
		using type = T;
	};

	class ApplicationContext {
	public:
		template <typename T>
		T* getSingleton(SingletonIdentifier<T> singIden) noexcept {
			return static_cast<T*>(singletons[singIden.name].get());
		}

		template <typename T, typename ... Params>
		void registerSingleton(SingletonIdentifier<T> singIden, Params ... params) noexcept {
			singletonInstancers.emplace_back([=]() {
				if (!prerequisitesReady(params...)) {
					return false;
				}

				singletons[singIden.name] = std::make_unique<T>(getParam(params)...);

				return true;
			});
		}

		bool instantiateSingletons() noexcept {
			auto progress = true;

			while (progress) {
				progress = false;

				for (auto iter = begin(singletonInstancers); iter != end(singletonInstancers); ) {
					if ((*iter)()) {
						iter = singletonInstancers.erase(iter);
						progress = true;
					} else {
						++iter;
					}
				}
			}

			return !singletonInstancers.size();
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<SingletonBase>> singletons;
		std::list<std::function<bool()>> singletonInstancers;

		template <typename T>
		T getParam(T param) noexcept {
			return param;
		}

		template <typename T>
		T* getParam(SingletonIdentifier<T> singIden) noexcept {
			return getSingleton(singIden);
		}

		bool prerequisitesReady() noexcept { return true; }

		template <typename T, typename ... Params>
		bool prerequisitesReady(T param, Params ... params) noexcept {
			return prerequisitesReady(params...);
		}

		template <typename T, typename ... Params>
		bool prerequisitesReady(SingletonIdentifier<T> singIden, Params ... params) noexcept {
			auto ready = getSingleton(singIden) != nullptr;

			if (ready) {
				return prerequisitesReady(params...);
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
