#include "summer.h"

#include <string>
#include <iostream>

class SampleConfig : public summer::SingletonBase {
public:
	SampleConfig(std::string path) {
		std::cout << "load: " << path << std::endl;
	}
};

class SampleSingleton : public summer::SingletonBase {
public:
	SampleSingleton(SampleConfig* config) {
		std::cout << "config address: " << config << std::endl;
	}
};

class SampleApplication : public summer::PrimarySource {
public:
	void setup(summer::ApplicationContext& context) noexcept {
		auto sampleConfig = S<SampleConfig>();
		auto sampleSingleton = S<SampleSingleton>();

		context.registerSingleton(sampleConfig, "hello");
		context.registerSingleton(sampleSingleton, sampleConfig);
	}

private:
	template <typename T>
	using S = summer::SingletonIdentifier<T>;
};

int main(int argc, char* argv[]) {
	summer::SummerApplication<SampleApplication>::run(argc, argv);

	return 0;
}
