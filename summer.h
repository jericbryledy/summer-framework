#ifndef _SUMMER_H
#define _SUMMER_H

#include <typeinfo>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace summer {

	class application_context;

	template <typename ModulePack>
	class context_support;

	class singleton_base {
	public:
		virtual ~singleton_base() noexcept {}

		virtual void post_construct(application_context& context) noexcept {}
	};

	template <typename SingletonType>
	class singleton_identifier {
	public:
		singleton_identifier() noexcept : singleton_identifier(typeid(SingletonType).name()) {}

		singleton_identifier(char const* name) noexcept : name_(name) {}
		singleton_identifier(std::string const& name) noexcept : name_(name) {}
		singleton_identifier(std::string&& name) noexcept : name_(std::move(name)) {}

		std::string const& name() const noexcept {
			return name_;
		}

	private:
		std::string name_;
	};

	template <typename Module, typename ... SingletonTypes>
	class module_base {
	public:
		template <typename Singleton>
		void register_singleton(Singleton& singleton) {
			call_register<Singleton, SingletonTypes...>(singleton);
		}

	private:
		template <typename Singleton, typename SingletonTypeArg = void, typename ... SingletonTypeArgs>
		void call_register(Singleton& singleton) {
			if constexpr (std::is_base_of_v<SingletonTypeArg, Singleton>) {
				static_cast<Module*>(this)->register_type(singleton);
			}

			if constexpr (sizeof...(SingletonTypeArgs) > 0) {
				call_register<Singleton, SingletonTypeArgs...>(singleton);
			}
		}
	};

	class application_context {
	public:

		template <typename SingletonType>
		SingletonType* get_singleton(singleton_identifier<SingletonType> const& singl_iden) noexcept {
			if (auto it = singletons.find(std::make_pair(std::type_index(typeid(SingletonType)), singl_iden.name())); it != singletons.end()) {
				return static_cast<SingletonType*>(it->second.get());
			}

			return nullptr;
		}

	private:

		using singleton_key = std::pair<std::type_index, std::string>;

		struct key_hasher {
			std::size_t operator()(singleton_key const& key) const {
				auto hash0 = std::get<0>(key).hash_code();
				auto hash1 = std::hash<std::string>{}(std::get<1>(key));

				// formula taken from boost::hash_combine
				return hash0 ^ (hash1 + 0x9e3779b9 + (hash0 << 6) + (hash0 >> 2));
			}
		};

		template <typename ModulePack>
		friend class context_support;

		std::unordered_map<singleton_key, std::unique_ptr<singleton_base>, key_hasher> singletons;
	};

	template <typename ModulePack>
	class context_support {
	public:
		context_support(application_context* context, ModulePack& modules) noexcept : context(context), modules(modules) {}

		template <typename SingletonType, typename ... Params>
		void register_singleton(singleton_identifier<SingletonType> const& singl_iden, Params const& ... params) noexcept {
			if (exists(singl_iden.name())) {
				std::cerr << "singleton [" << singl_iden.name() << "] already registered" << std::endl;

				return;
			}

			std::function<bool()> instantiator = [=]() {
				if (!prerequisites_ready(params...)) {
					return false;
				}

				auto[pair, success] = context->singletons.emplace(std::make_pair(std::type_index(typeid(SingletonType)), singl_iden.name()), std::make_unique<SingletonType>(get_param(params)...));
				auto singleton_ptr = static_cast<SingletonType*>(pair->second.get());

				register_to_modules.emplace_back([&, singleton_ptr]() {
					std::apply([&, singleton_ptr](auto&... modules) {
						(modules.register_singleton(*singleton_ptr), ...);
					}, modules);
				});

				return true;
			};

			std::function<void()> error_logger = [=]() {
				std::cerr << "error instantiating [" << singl_iden.name() << ']' << std::endl;
				print_missing(1, params...);
			};

			singleton_instancers[singl_iden.name()] = std::make_pair(instantiator, error_logger);
		}

		template <typename SingletonType>
		bool exists(singleton_identifier<SingletonType> const& singl_iden) {
			return exists(singl_iden.name());
		}

		bool exists(std::string const& name) {
			return singleton_instancers.find(name) != std::end(singleton_instancers);
		}

		bool instantiate_singletons() noexcept {
			for (auto progress = true; progress; ) {
				progress = false;

				for (auto iter = std::begin(singleton_instancers); iter != std::end(singleton_instancers); ) {
					if (std::get<0>(iter->second)()) {
						iter = singleton_instancers.erase(iter);
						progress = true;
					} else {
						++iter;
					}
				}
			}

			for (auto[name, instantiator_pair] : singleton_instancers) {
				auto[instantiator, error_logger] = instantiator_pair;

				error_logger();
			}

			return !singleton_instancers.size();
		}

		void do_post_constructs() noexcept {
			for (auto it = std::begin(context->singletons); it != std::end(context->singletons); ++it) {
				it->second->post_construct(*context);
			}
		}

		void register_singletons_to_modules() noexcept {
			for (auto& func : register_to_modules) {
				func();
			}

			register_to_modules.clear();
		}

	private:

		template <typename ParamType>
		ParamType& get_param(ParamType& param) noexcept {
			return param;
		}

		template <typename SingletonType>
		SingletonType& get_param(singleton_identifier<SingletonType> const& singl_iden) noexcept {
			return *context->get_singleton(singl_iden);
		}

		bool prerequisites_ready() noexcept { return true; }

		template <typename ParamType, typename ... ParamTypes>
		bool prerequisites_ready(ParamType const& param, ParamTypes const& ... params) noexcept {
			return prerequisites_ready(params...);
		}

		template <typename SingletonType, typename ... ParamTypes>
		bool prerequisites_ready(singleton_identifier<SingletonType> const& singl_iden, ParamTypes const& ... params) noexcept {
			if (context->get_singleton(singl_iden) != nullptr) {
				return prerequisites_ready(params...);
			}

			return false;
		}

		void print_missing(int) noexcept {}

		template <typename ParamType, typename ... ParamTypes>
		void print_missing(int index, ParamType const& param, ParamTypes const& ... params) noexcept {
			print_missing(index + 1, params...);
		}

		template <typename SingletonType, typename ... ParamTypes>
		void print_missing(int index, singleton_identifier<SingletonType> const& singl_iden, ParamTypes const& ... params) noexcept {
			if (context->get_singleton(singl_iden) == nullptr) {
				std::cerr << "  - param " << index << " [" << singl_iden.name() << "] missing" << std::endl;
			}

			print_missing(index + 1, params...);
		}

		ModulePack& modules;
		std::unordered_map<std::string, std::tuple<std::function<bool()>, std::function<void()>>> singleton_instancers;
		std::vector<std::function<void()>> register_to_modules;
		application_context* context;
	};

	template <typename ... Modules>
	class application_base {
	public:
		using module_pack = std::tuple<Modules...>;
		using context_support_t = context_support<module_pack>;

		void setup(context_support_t const& context_support) noexcept {
			std::cout << "default setup" << std::endl;
		}
	};

	template <typename SummerApplicationType>
	class summer_application {
	public:
		static void run(int argc, char* argv[]) noexcept {
			summer_application<SummerApplicationType> summer_app;

			auto args = std::vector<std::string>(argc);
			for (int i = 0; i < argc; ++i) {
				args.emplace_back(argv[i]);
			}

			summer_app.initialize(args);
		}

	private:
		summer_application() noexcept : context_support(&context, modules) {}

		void initialize(std::vector<std::string>& args) noexcept {
			summer_app.setup(context_support);

			initializeModules(args);

			context_support.instantiate_singletons();
			context_support.do_post_constructs();
			context_support.register_singletons_to_modules();
		}

		void initializeModules(std::vector<std::string>& args) noexcept {
			std::apply([this, &args](auto&... modules) {
				(modules.initialize(context_support, args), ...);
			}, modules);
		}

		SummerApplicationType summer_app;
		typename SummerApplicationType::module_pack modules;
		application_context context;
		typename SummerApplicationType::context_support_t context_support;

	};

}

#endif
