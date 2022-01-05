workspace "CppConveyor"
	location "build"
	platforms { "Console" }
	configurations { "Debug", "Release" }
	filter { "platforms:Console" }
		system "Windows"
		architecture "x86_64"
	filter{}

project "CppConveyor"
	location "build/CppConveyor"
	language "C++"
	filter {"platforms:Console"}
		kind "ConsoleApp"
		defines {
			"_CONSOLE"
		}
	filter {}
	files { 
		"src/**"
	}
	includedirs {
		"src",
		"src/**",
	}
	cppdialect "C++latest"