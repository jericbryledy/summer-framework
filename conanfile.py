from conans import ConanFile, CMake

class SummerFrameworkConan(ConanFile):
	generators = "cmake"

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()
