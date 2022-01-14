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
	debugdir "$(TargetDir)"
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
			'robocopy "' .. path.getabsolute("data/common") .. '" "$(TargetDir)data" /E',
		}
		if os.isdir(path.getabsolute('data/sdlPrivate')) then
			print("Private assets exist, using those")			
			postbuildcommands {
				'robocopy "' .. path.getabsolute("data/sdlPrivate") .. '" "$(TargetDir)data/platform" /E',
				'exit /b 0'
			}
		else			
			postbuildcommands {
				'robocopy "' .. path.getabsolute("data/sdl") .. '" "$(TargetDir)data/platform" /E',
				'exit /b 0'
			}
		end

		removefiles {
			"src/Platform/Console/**",
		}
		includedirs {
			"third_party/SDL/include",
			"third_party/SDLImage",
		}
		links {
			"SDL",
			"SDLImage",
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
		["data/*"] = "data/*",
		["data/*"] = "data/common/**",
		["data/platform/*"] = "data/console/**",
	}
	flags { "MultiProcessorCompile" }

dofile('third_party/sdl.lua')
dofile('third_party/sdlimage.lua')