#include "summer.h"
#include <iostream>

class SampleConfig : public summer::SingletonBase {
public:
	SampleConfig() {}
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
		auto sampleConfig = summer::SingletonIdentifier<SampleConfig>{"sampleConfig"};
		auto sampleSingleton = summer::SingletonIdentifier<SampleSingleton>{"sampleSingleton"};

		context.registerSingleton<SampleConfig>("sampleConfig");
		context.registerSingleton<SampleSingleton>("sampleSingleton", sampleConfig);
	}
};

int main(int argc, char* argv[]) {
	summer::SummerApplication<SampleApplication>::run(argc, argv);

	return 0;
}
