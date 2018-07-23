#include "summer.h"

#include <string>
#include <iostream>

class SampleConfig : public summer::SingletonBase {
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

class SampleSingleton : public summer::SingletonBase {
public:
	SampleSingleton(SampleConfig& config) {
		std::cout << "sample singleton path: " << config.getPath() << std::endl;
	}
};

class SampleApplication : public summer::PrimarySource {
public:
	void setup(summer::ApplicationContext& context) noexcept {
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
