require "ConveyorModule"

workspace "CppConveyor"
	location "build"
	platforms { --[["Console",]] "SDL" }
	configurations { "Debug", "Release" }
	filter { "platforms:Console" }
		system "Windows"
		architecture "x86_64"
	filter{}


    filter { "configurations:Release" }
   		optimize "Full"
		flags "NoRuntimeChecks"
		inlining "Auto"
		intrinsics "on"
	filter{}

group("Tools")
	project "AssetBuilder"
		kind "ConsoleApp"
		targetdir "bin/tools"
		targetname "AssetBuilder"
		location "build/AssetBuilder"
		language "C++"
		debugdir "$(TargetDir)"	
		files {
			"tools/AssetBuilder/src/**",
			"tools/AssetBuilder/include/**",
		}	
		includedirs {
			"tools/AssetBuilder/src/**",	
			"tools/AssetBuilder/include/**",		
		}
		flags { "FatalWarnings" }

group("")
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
		flags { "FatalWarnings" }

		if _ACTION == 'vs2022' then
			customtargets {
				[[
				  <Target Name="GenerateAssetSpec" BeforeTargets="InitializeBuildStatus">
				  	<Message Text="Generating Asset Specification" Importance="High" />
				  </Target>
				]]
			}
		end

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
				"third_party/SDL_Image",
				"third_party/imgui",
			}
			links {
				"SDL",
				"SDLImage",
				"imgui",
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

group("ThirdParty")
	dofile('third_party/sdl.lua')
	dofile('third_party/sdlimage.lua')
	dofile('third_party/imgui.lua')
