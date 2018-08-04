#include "summer.h"
#include "props.h"
#include "conversions.h"

#include <string>
#include <iostream>

class WebController {};

class WebModule : public summer::ModuleBase<WebModule, WebController>{
public:
	void initialize(std::vector<std::string>& args) {
		std::cout << "web module initialize!" << std::endl;
	}

	void registerType(WebController& webcontroller) {
		std::cout << "web controller registered!" << std::endl;
	}
};

class ModuleB : public summer::ModuleBase<ModuleB>{
public:
	void initialize(std::vector<std::string>& args) {
		std::cout << "module b initialize!" << std::endl;
	}
};

class ModuleC : public summer::ModuleBase<ModuleC> {
public:
	void initialize(std::vector<std::string>& args) {
		std::cout << "module c initialize!" << std::endl;
	}
};

using SampleApplicationType = summer::ApplicationBase<WebModule, ModuleB, ModuleC>;

class SampleConfig : public SampleApplicationType::Singleton {
public:
	SampleConfig(std::string path) : path(path) {
		std::cout << "load: " << path << std::endl;
	}

	const std::string& getPath() {
		return path;
	}
private:
	std::string path;
};

class SampleSingleton : public SampleApplicationType::Singleton, public WebController {
public:
	SampleSingleton(SampleConfig& config) {
		std::cout << "sample singleton path: " << config.getPath() << std::endl;
	}

	void postConstruct(SampleApplicationType::Context& context) noexcept override {
		config = context.getSingleton(summer::SingletonIdentifier<SampleConfig>());

		std::cout << "i got: " << config << std::endl;
	}

private:
	SampleConfig * config;
};

class SampleApplication : public SampleApplicationType {
public:
	void setup(Context& context) noexcept {
		auto sampleConfig = S<SampleConfig>();

		context.registerSingleton(sampleConfig, "hello");
		context.registerSingleton(S<SampleSingleton>(), sampleConfig);
	}

private:
	template <typename T>
	using S = summer::SingletonIdentifier<T>;
};

int main(int argc, char* argv[]) {
	summer::SummerApplication<SampleApplication>::run(argc, argv);

	return 0;
}
