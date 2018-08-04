#ifndef _SUMMER_H
#define _SUMMER_H

#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace summer {

	template <typename Application>
	class ContextBase;

	template <typename Application>
	class SingletonBase {
	public:
		virtual ~SingletonBase() noexcept {}

		virtual void postConstruct(typename Application::Context& context) noexcept {}
	};

	template <typename SingletonType>
	class SingletonIdentifier {
	public:
		SingletonIdentifier() noexcept : SingletonIdentifier(typeid(SingletonType).name()) {}

		SingletonIdentifier(const char* name) noexcept : name(name) {}
		SingletonIdentifier(const std::string& name) noexcept : name(name) {}
		SingletonIdentifier(std::string&& name) noexcept : name(std::move(name)) {}

		const std::string& getName() const noexcept {
			return name;
		}

	private:
		std::string name;
	};

	template <typename Module, typename ... SingletonTypes>
	class ModuleBase {
	public:
		template <typename Singleton>
		void registerSingleton(Singleton& singleton) {
			callRegister<Singleton, SingletonTypes...>(singleton);
		}

	private:
		template <typename Singleton, typename SingletonTypeArg = void, typename ... SingletonTypeArgs>
		void callRegister(Singleton& singleton) {
			if constexpr (std::is_base_of_v<SingletonTypeArg, Singleton>) {
				static_cast<Module*>(this)->registerType(singleton);
			}

			if constexpr (sizeof...(SingletonTypeArgs) > 0) {
				callRegister<Singleton, SingletonTypeArgs...>(singleton);
			}
		}
	};

	template <typename Application>
	class ContextBase {
	public:
		ContextBase(typename Application::ModulePack& modules) noexcept : modules(modules) {}

		template <typename SingletonType>
		SingletonType* getSingleton(const SingletonIdentifier<SingletonType>& singIden) noexcept {
			if (auto it = singletons.find(std::make_pair(std::type_index(typeid(SingletonType)), singIden.getName())); it != singletons.end()) {
				return static_cast<SingletonType*>(it->second.get());
			}

			return nullptr;
		}

		template <typename SingletonType, typename ... Params>
		void registerSingleton(const SingletonIdentifier<SingletonType>& singIden, const Params& ... params) noexcept {
			if (singletonInstancers.find(singIden.getName()) != std::end(singletonInstancers)) {
				std::cerr << "singleton [" << singIden.getName() << "] already registered" << std::endl;

				return;
			}

			std::function<bool()> instantiator = [=]() {
				if (!prerequisitesReady(params...)) {
					return false;
				}

				auto[pair, success] = singletons.emplace(std::make_pair(std::type_index(typeid(SingletonType)), singIden.getName()), std::make_unique<SingletonType>(getParam(params)...));
				auto pSingleton = static_cast<SingletonType*>(pair->second.get());

				registerToModules.emplace_back([&, pSingleton]() {
					std::apply([&, pSingleton](auto&... modules) {
						(modules.registerSingleton(*pSingleton), ...);
					}, modules);
				});

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

			for (auto[name, instantiatorPair] : singletonInstancers) {
				auto[instantiator, errorLogger] = instantiatorPair;

				errorLogger();
			}

			return !singletonInstancers.size();
		}

		void doPostConstructs() noexcept {
			for (auto it = std::begin(singletons); it != std::end(singletons); ++it) {
				it->second->postConstruct(*this);
			}
		}

		void registerSingletonsToModules() noexcept {
			for (auto& func : registerToModules) {
				func();
			}

			registerToModules.clear();
		}

	private:

		template <typename ParamType>
		ParamType& getParam(ParamType& param) noexcept {
			return param;
		}

		template <typename SingletonType>
		SingletonType& getParam(const SingletonIdentifier<SingletonType>& singIden) noexcept {
			return *getSingleton(singIden);
		}

		bool prerequisitesReady() noexcept { return true; }

		template <typename ParamType, typename ... ParamTypes>
		bool prerequisitesReady(const ParamType& param, const ParamTypes& ... params) noexcept {
			return prerequisitesReady(params...);
		}

		template <typename SingletonType, typename ... ParamTypes>
		bool prerequisitesReady(const SingletonIdentifier<SingletonType>& singIden, const ParamTypes& ... params) noexcept {
			if (getSingleton(singIden) != nullptr) {
				return prerequisitesReady(params...);
			}

			return false;
		}

		void printMissing(int) noexcept {}

		template <typename ParamType, typename ... ParamTypes>
		void printMissing(int index, const ParamType& param, const ParamTypes& ... params) noexcept {
			printMissing(index + 1, params...);
		}

		template <typename SingletonType, typename ... ParamTypes>
		void printMissing(int index, const SingletonIdentifier<SingletonType>& singIden, const ParamTypes& ... params) noexcept {
			if (getSingleton(singIden) == nullptr) {
				std::cerr << "  - param " << index << " [" << singIden.getName() << "] missing" << std::endl;
			}

			printMissing(index + 1, params...);
		}

		using SingletonKey = std::pair<std::type_index, std::string>;

		struct KeyHasher {
			std::size_t operator()(const SingletonKey& key) const {
				auto hash0 = std::get<0>(key).hash_code();
				auto hash1 = std::hash<std::string>{}(std::get<1>(key));

				// formula taken from boost::hash_combine
				return hash0 ^ (hash1 + 0x9e3779b9 + (hash0 << 6) + (hash0 >> 2));
			}
		};

		typename Application::ModulePack& modules;
		std::unordered_map<SingletonKey, std::unique_ptr<typename Application::Singleton>, KeyHasher> singletons;
		std::unordered_map<std::string, std::tuple<std::function<bool()>, std::function<void()>>> singletonInstancers;
		std::vector<std::function<void()>> registerToModules;
	};

	template <typename ... Modules>
	class ApplicationBase {
	public:
		using ModulePack = std::tuple<Modules...>;
		using Context = ContextBase<ApplicationBase<Modules...>>;
		using Singleton = SingletonBase<ApplicationBase<Modules...>>;

		void setup(const Context& context) noexcept {
			std::cout << "default setup" << std::endl;
		}
	};

	template <typename SummerApplicationType>
	class SummerApplication {
	public:
		static void run(int argc, char* argv[]) noexcept {
			SummerApplication<SummerApplicationType> summerApp;

			auto args = std::vector<std::string>(argc);
			for (int i = 0; i < argc; ++i) {
				args.emplace_back(argv[i]);
			}

			summerApp.initialize(args);
		}

	private:
		SummerApplication() noexcept : context(modules) {}

		void initialize(std::vector<std::string>& args) noexcept {
			initializeModules(args);

			summerApp.setup(context);
			context.instantiateSingletons();
			context.doPostConstructs();
			context.registerSingletonsToModules();
		}

		void initializeModules(std::vector<std::string>& args) noexcept {
			std::apply([&args](auto&... modules) {
				(modules.initialize(args), ...);
			}, modules);
		}

		SummerApplicationType summerApp;
		typename SummerApplicationType::ModulePack modules;
		typename SummerApplicationType::Context context;

	};

}

#endif
