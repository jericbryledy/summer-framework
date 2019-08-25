#include "summer.h"
#include "summer/web.h"
#include "props.h"
#include "conversions.h"

#include <string>
#include <iostream>

class module_b : public summer::module_base<module_b> {
public:
	template <typename ContextSupport>
	void initialize(ContextSupport& context, std::vector<std::string>& args) {
		std::cout << "module b initialize!" << std::endl;
	}
};

class module_c : public summer::module_base<module_c> {
public:
	template <typename ContextSupport>
	void initialize(ContextSupport& context, std::vector<std::string>& args) {
		std::cout << "module c initialize!" << std::endl;
	}
};

using sample_application_type = summer::application_base<summer::modules::web::web_module, module_b, module_c>;

class sample_config : public summer::singleton_base {
public:
	sample_config(std::string path) : path_(path) {
		std::cout << "load: " << path << std::endl;
	}

	std::string const& path() {
		return path_;
	}
private:
	std::string path_;
};

class sample_singleton : public summer::singleton_base, public summer::modules::web::controller {
public:
	sample_singleton(std::shared_ptr<sample_config> config) {
		std::cout << "sample singleton path: " << config->path() << std::endl;
	}

	void post_construct(summer::application_context& context) noexcept override {
		config = context.get_singleton(summer::singleton_identifier<sample_config>());

		std::cout << "i got: " << config << std::endl;
	}

private:
	std::shared_ptr<sample_config> config;
};

class sample_application : public sample_application_type {
public:
	void setup(context_support_t& context_support) noexcept {
		auto sample_config__ = s<sample_config>();

		context_support.register_singleton(sample_config__, "hello");
		context_support.register_singleton(s<sample_singleton>(), sample_config__);
	}

private:
	template <typename T>
	using s = summer::singleton_identifier<T>;
};

int main(int argc, char* argv[]) {
	summer::summer_application<sample_application>::run(argc, argv);

	return 0;
}
