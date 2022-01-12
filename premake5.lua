workspace "CppConveyor"
	location "build"
	platforms { "Console", "SDL" }
	configurations { "Debug", "Release" }
	filter { "platforms:Console" }
		system "Windows"
		architecture "x86_64"
	filter{}

project "CppConveyor"
	location "build/CppConveyor"
	language "C++"
	editandcontinue "On"
	files { 
		".editorconfig",
		"data/**",
		"src/**"
	}

	filter {"platforms:Console"}
		kind "ConsoleApp"
		defines {
			"_CONSOLE"
		}
		postbuildcommands {
			'robocopy "' .. path.getabsolute("data/console") .. '" "$(TargetDir)data/platform" /E',
			'robocopy "' .. path.getabsolute("data/common") .. '" "$(TargetDir)data" /E',
			'exit /b 0'
		}
		removefiles {
			"src/Platform/SDL/**",
		}
	filter {"platforms:SDL"}
		kind "WindowedApp"
		defines {
			"_SDL"
		}
		postbuildcommands {
			'robocopy "' .. path.getabsolute("data/sdl") .. '" "$(TargetDir)data/platform" /E',
			'robocopy "' .. path.getabsolute("data/common") .. '" "$(TargetDir)data" /E',
			'exit /b 0'
		}
		removefiles {
			"src/Platform/Console/**",
		}
	filter {}
	includedirs {
		"src",
		"src/**",
	}
	cppdialect "C++latest"
	vpaths
	{
		["*"] = "src",
		["data/*"] = "data/common/**",
		["data/platform/*"] = "data/console/**",
	}
