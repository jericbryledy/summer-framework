#include "summer.h"
#include "props.h"
#include "conversions.h"

#include <string>
#include <iostream>

class web_controller {};

class web_module : public summer::module_base<web_module, web_controller>{
public:
	void initialize(std::vector<std::string>& args) {
		std::cout << "web module initialize!" << std::endl;
	}

	void register_type(web_controller& web_controller) {
		std::cout << "web controller registered!" << std::endl;
	}
};

class module_b : public summer::module_base<module_b>{
public:
	void initialize(std::vector<std::string>& args) {
		std::cout << "module b initialize!" << std::endl;
	}
};

class module_c : public summer::module_base<module_c> {
public:
	void initialize(std::vector<std::string>& args) {
		std::cout << "module c initialize!" << std::endl;
	}
};

using sample_application_type = summer::application_base<web_module, module_b, module_c>;

class sample_config : public sample_application_type::singleton {
public:
	sample_config(std::string path) : path_(path) {
		std::cout << "load: " << path << std::endl;
	}

	const std::string& path() {
		return path_;
	}
private:
	std::string path_;
};

class sample_singleton : public sample_application_type::singleton, public web_controller {
public:
	sample_singleton(sample_config& config) {
		std::cout << "sample singleton path: " << config.path() << std::endl;
	}

	void post_construct(sample_application_type::context& context) noexcept override {
		config_ = context.get_singleton(summer::singleton_identifier<sample_config>());

		std::cout << "i got: " << config_ << std::endl;
	}

private:
	sample_config * config_;
};

class sample_application : public sample_application_type {
public:
	void setup(context& context) noexcept {
		auto sample_config__ = s<sample_config>();

		context.register_singleton(sample_config__, "hello");
		context.register_singleton(s<sample_singleton>(), sample_config__);
	}

private:
	template <typename T>
	using s = summer::singleton_identifier<T>;
};

int main(int argc, char* argv[]) {
	summer::summer_application<sample_application>::run(argc, argv);

	return 0;
}
