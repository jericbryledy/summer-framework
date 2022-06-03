# Introduction

Summer Framework is a header only, dependency injection framework written in C++.
Think Spring Framework, but everything is figured out during compile time.
This is possible through C++'s powerful template parameter pack expansion.

The core system is made with modularity in mind.
Each application may have multiple modules.
Each module may have multiple singleton registered to it.
Singletons are not exclusive to the module it is registered in.
Hence, a singleton may be accessed by another module.

By default, a singleton's name will be `typeid(SingletonType).name()`.
Keep in mind that this means the default name is compiler dependent.
If one needs to access a dependency by its name, it would be safer to set the name yourself.
The default name should work just fine if one intend to access by class type.

# Building

The project uses [Conan Package Manager](https://conan.io/).
Create a directory named `build` and follow the following instructions inside the created directory:
1. Install dependencies with: `conan install ..`
2. Prepare the CMake files.
	* `cmake ..` (Generic)
	* `cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release` (for Linux release)
	* `cmake .. -G "Visual Studio 16"` (for Windows)
3. Build the project
	* `cmake --build .` (Generic)
	* `cmake --build . --config Release` (for Windows release)
4. Binary can be found in `build/bin/main`

For more information about building in Conan, visit https://docs.conan.io/en/latest/getting_started.html

# Future work

A web module would be nice ¯\\\_(ツ)_/¯.
